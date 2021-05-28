/* MollenOS
 *
 * Copyright 2020, Philip Meulengracht
 *
 * This program is free software : you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation ? , either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Vioarr - Vali Compositor
 * - Implements the default system compositor for Vali. It utilizies the gracht library
 *   for communication between compositor clients and the server. The server renders
 *   using Mesa3D with either the soft-renderer or llvmpipe render for improved performance.
 */

#include <errno.h>
#include <gracht/link/socket.h>
#include <gracht/server.h>
#include <gracht/client.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef MOLLENOS
#include <ddk/service.h>
#include <ddk/utils.h>
#include <gracht/link/vali.h>
#include <io.h>
#include <ioset.h>
#include <os/process.h>

#include "ctt_input_service_client.h"
#include "svc_device_service_client.h"
#elif defined(_WIN32)
#include <gracht/link/socket.h>
#include <windows.h>
static const char* g_ipAddress = "127.0.0.1";
static uint16_t    g_portNo    = 55555;
#else
#include <gracht/link/socket.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <sys/socket.h>
static const char* g_serverPath = "/tmp/vi-srv";
#endif

#include "wm_core_service_server.h"
#include "wm_screen_service_server.h"
#include "wm_memory_service_server.h"
#include "wm_memory_pool_service_server.h"
#include "wm_buffer_service_server.h"
#include "wm_surface_service_server.h"
#include "wm_pointer_service_server.h"

#include "engine/vioarr_engine.h"
#include "engine/vioarr_objects.h"
#include "engine/vioarr_utils.h"

static gracht_server_t* g_valiServer = NULL;

gracht_server_t* vioarr_get_server_handle(void)
{
    return g_valiServer;
}

static void __gracht_handle_disconnect(int client)
{
    vioarr_objects_remove_by_client(client);
}

#ifdef MOLLENOS
static int __create_platform_link(void)
{


    return 0;
}
#elif defined(_WIN32)
static int __create_platform_link(void)
{
    struct gracht_link_socket* link;
    struct sockaddr_in         addr = { 0 };

    // initialize the WSA library
    gracht_link_socket_setup();

    // AF_INET is the Internet address family.
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(g_ipAddress);
    addr.sin_port = htons(g_portNo);

    gracht_link_socket_create(&link);
    gracht_link_socket_set_type(link, gracht_link_stream_based);
    gracht_link_socket_set_address(link, (const struct sockaddr_storage*)&addr, sizeof(struct sockaddr_un));
    gracht_link_socket_set_domain(link, AF_INET);
    gracht_link_socket_set_listen(link, 1);
    
    return gracht_server_add_link(g_valiServer, (struct gracht_link*)link);
}
#else
static int __create_platform_link(void)
{
    struct gracht_link_socket* link;
    struct sockaddr_un         addr = { 0 };

    // delete any preexisting file
    unlink(g_serverPath);

    addr.sun_family = AF_LOCAL;
    strncpy (addr.sun_path, g_serverPath, sizeof(addr.sun_path));
    addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';

    gracht_link_socket_create(&link);
    gracht_link_socket_set_type(link, gracht_link_stream_based);
    gracht_link_socket_set_address(link, (const struct sockaddr_storage*)&addr, sizeof(struct sockaddr_un));
    gracht_link_socket_set_domain(link, AF_LOCAL);
    gracht_link_socket_set_listen(link, 1);

    return gracht_server_add_link(g_valiServer, (struct gracht_link*)link);
}
#endif

#if defined(MOLLENOS)
static gracht_client_t* g_valiClient = NULL;

int server_initialize(int* eventIodOut)
{
    struct gracht_server_configuration config;
    int                                status;
    
    gracht_server_configuration_init(&config);

    // Create the set descriptor we are listening to
    gracht_server_configuration_set_aio_descriptor(&config, ioset(0));
    if (config.set_descriptor < 0) {
        vioarr_utils_error("error creating event descriptor %i", errno);
        return -1;
    }

    // Listen to client disconnects so we can remove resources
    config.callbacks.clientDisconnected = __gracht_handle_disconnect;
    
    status = gracht_server_create(&config, &g_valiServer);
    if (status) {
        vioarr_utils_error("error initializing server library %i", errno);
        close(config.set_descriptor);
    }

    // create the platform link
    status = __create_platform_link();
    if (status) {
        vioarr_utils_error("error initializing server link %i", errno);
        close(config.set_descriptor);
    }

    *eventIodOut = config.set_descriptor;
    return status;
}

int client_initialize(void)
{
    struct gracht_client_configuration clientConfiguration;
    int                                status;

    status = gracht_link_vali_client_create(&clientConfiguration.link);
    if (status) {
        return status;
    }

    status = gracht_client_create(&clientConfiguration, &g_valiClient);
    if (status) {
        return status;
    }
    return status;
}

void server_get_hid_devices(void)
{
    struct vali_link_message msg = VALI_MSG_INIT_HANDLE(GetDeviceService());
    vioarr_utils_trace("[server_get_hid_devices]");

    // subscribe to events from the device manager
    svc_device_subscribe(g_valiClient, &msg.base);

    // query all input devices
    svc_device_get_devices_by_protocol(g_valiClient, &msg.base, PROTOCOL_CTT_INPUT_ID);
}

void svc_device_event_protocol_device_callback(struct svc_device_protocol_device_event* input)
{
    struct vali_link_message msg = VALI_MSG_INIT_HANDLE(input->driver_id);
    vioarr_utils_trace("[svc_device_event_protocol_device_callback] %u", input->device_id);

    // subscribe to the driver
    ctt_input_subscribe(g_valiClient, &msg.base);

    // get properties of the device
    ctt_input_get_properties(g_valiClient, &msg.base, input->device_id);
}

void svc_device_event_device_update_callback(struct svc_device_device_update_event* input)
{
    // todo handle connection of new devices and disconnection
}

int server_run(int eventIod)
{
    struct ioset_event events[32];
    int                serverRunning = 1;
    int                clientIod;

    // listen to client events as well
    clientIod = gracht_client_iod(g_valiClient);
    ioset_ctrl(eventIod, IOSET_ADD, clientIod,
               &(struct ioset_event) {
                       .data.iod = clientIod,
                       .events   = IOSETIN | IOSETCTL | IOSETLVT
               });

    // Initialize the chain of events by retrieving all input devices
    server_get_hid_devices();

#ifdef VIOARR_LAUNCHER
    UUId_t pid;
    ProcessSpawn("$bin/" VIOARR_LAUNCHER, NULL, &pid);
#endif //VIOARR_LAUNCHER

    // Server main loop
    while (serverRunning) {
        int num_events = ioset_wait(eventIod, &events[0], 32, 0);
        for (int i = 0; i < num_events; i++) {
            if (events[i].data.iod == clientIod) {
                gracht_client_wait_message(g_valiClient, NULL, 0);
            }
            else {
                gracht_server_handle_event(events[i].data.iod, events[i].events);
            }
        }
    }
    return 0;
}

/*******************************************
 * Entry Point
 *******************************************/
int main(int argc, char **argv)
{
    int eventIod;

#if !defined(VIOARR_TRACEMODE)
    MollenOSEndBoot();
#endif

    int status = client_initialize();
    if (status) {
        return status;
    }
    
    status = server_initialize(&eventIod);
    if (status) {
        return status;
    }
    
    status = vioarr_engine_initialize();
    if (status) {
        return status;
    }

    // add the protocols we would like to support
    gracht_client_register_protocol(g_valiClient, &svc_device_client_protocol);
    gracht_client_register_protocol(g_valiClient, &ctt_input_client_protocol);    

    // add the server protocols we support
    gracht_server_register_protocol(g_valiServer, &wm_core_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_screen_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_memory_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_memory_pool_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_buffer_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_surface_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_pointer_server_protocol);

    return server_run(eventIod);
}
#elif defined(_WIN32)

int server_initialize(int* eventIodOut)
{
    struct gracht_server_configuration config;
    int                                status;
    
    gracht_server_configuration_init(&config);




    // Listen to client disconnects so we can remove resources
    config.callbacks.clientDisconnected = __gracht_handle_disconnect;
    
    status = gracht_server_create(&config, &g_valiServer);
    if (status) {
        vioarr_utils_error("error initializing server library %i", errno);
        close(config.set_descriptor);
    }

    // create the platform link
    status = __create_platform_link();
    if (status) {
        vioarr_utils_error("error initializing server link %i", errno);
        close(config.set_descriptor);
    }

    *eventIodOut = config.set_descriptor;
    return status;
}

/*******************************************
 * Entry Point
 *******************************************/
int main(int argc, char **argv)
{
    int eventIod;

    int status = client_initialize();
    if (status) {
        return status;
    }
    
    status = server_initialize(&eventIod);
    if (status) {
        return status;
    }
    
    status = vioarr_engine_initialize();
    if (status) {
        return status;
    }

    // add the server protocols we support
    gracht_server_register_protocol(g_valiServer, &wm_core_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_screen_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_memory_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_memory_pool_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_buffer_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_surface_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_pointer_server_protocol);

    return server_run(eventIod);
}
#else

int server_initialize(void)
{
    struct gracht_server_configuration config;
    int                                status;
    
    gracht_server_configuration_init(&config);

    // Create the set descriptor we are listening to
    gracht_server_configuration_set_aio_descriptor(&config, epoll_create1(0));
    if (config.set_descriptor < 0) {
        vioarr_utils_error("error creating event descriptor %i\n", errno);
        return -1;
    }

    // Listen to client disconnects so we can remove resources
    config.callbacks.clientDisconnected = __gracht_handle_disconnect;
    
    status = gracht_server_create(&config, &g_valiServer);
    if (status) {
        vioarr_utils_error("error initializing server library %i\n", errno);
        close(config.set_descriptor);
    }

    // create the platform link
    status = __create_platform_link();
    if (status) {
        vioarr_utils_error("error initializing server link %i\n", errno);
        close(config.set_descriptor);
    }
    return status;
}

/*******************************************
 * Entry Point
 *******************************************/
int main(int argc, char **argv)
{
    int status = server_initialize();
    if (status) {
        return status;
    }
    
    status = vioarr_engine_initialize();
    if (status) {
        return status;
    }

    // not used
    (void)argc;
    (void)argv;
    
    // add the server protocols we support
    gracht_server_register_protocol(g_valiServer, &wm_core_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_screen_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_memory_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_memory_pool_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_buffer_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_surface_server_protocol);
    gracht_server_register_protocol(g_valiServer, &wm_pointer_server_protocol);
    return gracht_server_main_loop(g_valiServer);
}

#endif
