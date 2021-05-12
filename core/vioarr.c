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
#include <ddk/service.h>
#include <ddk/utils.h>
#include <gracht/link/socket.h>
#include <gracht/link/vali.h>
#include <gracht/server.h>
#include <io.h>
#include <ioset.h>
#include <os/process.h>
#include <stdio.h>
#include <stdlib.h>

#include "protocols/ctt_input_protocol_client.h"
#include "protocols/svc_device_protocol_client.h"

#include "protocols/wm_core_protocol_server.h"
#include "protocols/wm_screen_protocol_server.h"
#include "protocols/wm_memory_protocol_server.h"
#include "protocols/wm_memory_pool_protocol_server.h"
#include "protocols/wm_buffer_protocol_server.h"
#include "protocols/wm_surface_protocol_server.h"
#include "protocols/wm_pointer_protocol_server.h"

#include "engine/vioarr_engine.h"
#include "engine/vioarr_objects.h"
#include "engine/vioarr_utils.h"

extern void ctt_input_event_properties_callback(struct ctt_input_properties_event*);
extern void ctt_input_event_button_callback(struct ctt_input_button_event*);
extern void ctt_input_event_cursor_callback(struct ctt_input_cursor_event*);
 
static gracht_protocol_function_t ctt_input_callbacks[3] = {
    { PROTOCOL_CTT_INPUT_EVENT_PROPERTIES_ID , ctt_input_event_properties_callback },
    { PROTOCOL_CTT_INPUT_EVENT_BUTTON_ID , ctt_input_event_button_callback },
    { PROTOCOL_CTT_INPUT_EVENT_CURSOR_ID , ctt_input_event_cursor_callback },
};
DEFINE_CTT_INPUT_CLIENT_PROTOCOL(ctt_input_callbacks, 3);

static void svc_device_event_protocol_device_callback(struct svc_device_protocol_device_event*);
static void svc_device_event_device_update_callback(struct svc_device_device_update_event*);
 
static gracht_protocol_function_t svc_device_callbacks[2] = {
    { PROTOCOL_SVC_DEVICE_EVENT_PROTOCOL_DEVICE_ID , svc_device_event_protocol_device_callback },
    { PROTOCOL_SVC_DEVICE_EVENT_DEVICE_UPDATE_ID , svc_device_event_device_update_callback },
};
DEFINE_SVC_DEVICE_CLIENT_PROTOCOL(svc_device_callbacks, 2);

extern void wm_core_sync_callback(struct gracht_recv_message* message, struct wm_core_sync_args*);
extern void wm_core_get_objects_callback(struct gracht_recv_message* message);

static gracht_protocol_function_t wm_core_callbacks[2] = {
    { PROTOCOL_WM_CORE_SYNC_ID , wm_core_sync_callback },
    { PROTOCOL_WM_CORE_GET_OBJECTS_ID , wm_core_get_objects_callback },
};
DEFINE_WM_CORE_SERVER_PROTOCOL(wm_core_callbacks, 2);

extern void wm_screen_get_properties_callback(struct gracht_recv_message* message, struct wm_screen_get_properties_args*);
extern void wm_screen_get_modes_callback(struct gracht_recv_message* message, struct wm_screen_get_modes_args*);
extern void wm_screen_set_scale_callback(struct gracht_recv_message* message, struct wm_screen_set_scale_args*);
extern void wm_screen_set_transform_callback(struct gracht_recv_message* message, struct wm_screen_set_transform_args*);
extern void wm_screen_create_surface_callback(struct gracht_recv_message* message, struct wm_screen_create_surface_args*);

static gracht_protocol_function_t wm_screen_callbacks[5] = {
    { PROTOCOL_WM_SCREEN_GET_PROPERTIES_ID , wm_screen_get_properties_callback },
    { PROTOCOL_WM_SCREEN_GET_MODES_ID , wm_screen_get_modes_callback },
    { PROTOCOL_WM_SCREEN_SET_SCALE_ID , wm_screen_set_scale_callback },
    { PROTOCOL_WM_SCREEN_SET_TRANSFORM_ID , wm_screen_set_transform_callback },
    { PROTOCOL_WM_SCREEN_CREATE_SURFACE_ID , wm_screen_create_surface_callback },
};
DEFINE_WM_SCREEN_SERVER_PROTOCOL(wm_screen_callbacks, 5);

extern void wm_memory_create_pool_callback(struct gracht_recv_message* message, struct wm_memory_create_pool_args*);

static gracht_protocol_function_t wm_memory_callbacks[1] = {
    { PROTOCOL_WM_MEMORY_CREATE_POOL_ID , wm_memory_create_pool_callback },
};
DEFINE_WM_MEMORY_SERVER_PROTOCOL(wm_memory_callbacks, 1);

extern void wm_memory_pool_create_buffer_callback(struct gracht_recv_message* message, struct wm_memory_pool_create_buffer_args*);
extern void wm_memory_pool_destroy_callback(struct gracht_recv_message* message, struct wm_memory_pool_destroy_args*);

static gracht_protocol_function_t wm_memory_pool_callbacks[2] = {
    { PROTOCOL_WM_MEMORY_POOL_CREATE_BUFFER_ID , wm_memory_pool_create_buffer_callback },
    { PROTOCOL_WM_MEMORY_POOL_DESTROY_ID ,       wm_memory_pool_destroy_callback }
};
DEFINE_WM_MEMORY_POOL_SERVER_PROTOCOL(wm_memory_pool_callbacks, 2);

extern void wm_buffer_destroy_callback(struct gracht_recv_message* message, struct wm_buffer_destroy_args*);

static gracht_protocol_function_t wm_buffer_callbacks[1] = {
    { PROTOCOL_WM_BUFFER_DESTROY_ID , wm_buffer_destroy_callback },
};
DEFINE_WM_BUFFER_SERVER_PROTOCOL(wm_buffer_callbacks, 1);

extern void wm_surface_get_formats_callback(struct gracht_recv_message* message, struct wm_surface_get_formats_args*);
extern void wm_surface_set_buffer_callback(struct gracht_recv_message* message, struct wm_surface_set_buffer_args*);
extern void wm_surface_invalidate_callback(struct gracht_recv_message* message, struct wm_surface_invalidate_args*);
extern void wm_surface_set_transparency_callback(struct gracht_recv_message* message, struct wm_surface_set_transparency_args*);
extern void wm_surface_set_drop_shadow_callback(struct gracht_recv_message* message, struct wm_surface_set_drop_shadow_args*);
extern void wm_surface_set_input_region_callback(struct gracht_recv_message* message, struct wm_surface_set_input_region_args*);
extern void wm_surface_add_subsurface_callback(struct gracht_recv_message* message, struct wm_surface_add_subsurface_args*);
extern void wm_surface_resize_subsurface_callback(struct gracht_recv_message* message, struct wm_surface_resize_subsurface_args*);
extern void wm_surface_move_subsurface_callback(struct gracht_recv_message* message, struct wm_surface_move_subsurface_args*);
extern void wm_surface_request_fullscreen_mode(struct gracht_recv_message* message, struct wm_surface_request_fullscreen_mode_args*);
extern void wm_surface_request_level(struct gracht_recv_message* message, struct wm_surface_request_level_args*);
extern void wm_surface_request_frame_callback(struct gracht_recv_message* message, struct wm_surface_request_frame_args*);
extern void wm_surface_commit_callback(struct gracht_recv_message* message, struct wm_surface_commit_args*);
extern void wm_surface_resize_callback(struct gracht_recv_message* message, struct wm_surface_resize_args*);
extern void wm_surface_move_callback(struct gracht_recv_message* message, struct wm_surface_move_args*);
extern void wm_surface_destroy_callback(struct gracht_recv_message* message, struct wm_surface_destroy_args*);

static gracht_protocol_function_t wm_surface_callbacks[16] = {
    { PROTOCOL_WM_SURFACE_GET_FORMATS_ID , wm_surface_get_formats_callback },
    { PROTOCOL_WM_SURFACE_SET_BUFFER_ID , wm_surface_set_buffer_callback },
    { PROTOCOL_WM_SURFACE_INVALIDATE_ID , wm_surface_invalidate_callback },
    { PROTOCOL_WM_SURFACE_SET_TRANSPARENCY_ID , wm_surface_set_transparency_callback },
    { PROTOCOL_WM_SURFACE_SET_DROP_SHADOW_ID , wm_surface_set_drop_shadow_callback },
    { PROTOCOL_WM_SURFACE_SET_INPUT_REGION_ID , wm_surface_set_input_region_callback },
    { PROTOCOL_WM_SURFACE_ADD_SUBSURFACE_ID , wm_surface_add_subsurface_callback },
    { PROTOCOL_WM_SURFACE_RESIZE_SUBSURFACE_ID , wm_surface_resize_subsurface_callback },
    { PROTOCOL_WM_SURFACE_MOVE_SUBSURFACE_ID , wm_surface_move_subsurface_callback },
    { PROTOCOL_WM_SURFACE_REQUEST_FULLSCREEN_MODE_ID, wm_surface_request_fullscreen_mode },
    { PROTOCOL_WM_SURFACE_REQUEST_LEVEL_ID, wm_surface_request_level },
    { PROTOCOL_WM_SURFACE_REQUEST_FRAME_ID , wm_surface_request_frame_callback },
    { PROTOCOL_WM_SURFACE_COMMIT_ID , wm_surface_commit_callback },
    { PROTOCOL_WM_SURFACE_RESIZE_ID , wm_surface_resize_callback },
    { PROTOCOL_WM_SURFACE_MOVE_ID , wm_surface_move_callback },
    { PROTOCOL_WM_SURFACE_DESTROY_ID , wm_surface_destroy_callback },
};
DEFINE_WM_SURFACE_SERVER_PROTOCOL(wm_surface_callbacks, 16);

extern void wm_pointer_set_surface_callback(struct gracht_recv_message* message, struct wm_pointer_set_surface_args*);
extern void wm_pointer_grab_callback(struct gracht_recv_message* message, struct wm_pointer_grab_args*);
extern void wm_pointer_ungrab_callback(struct gracht_recv_message* message, struct wm_pointer_ungrab_args*);

static gracht_protocol_function_t wm_pointer_callbacks[3] = {
    { PROTOCOL_WM_POINTER_SET_SURFACE_ID , wm_pointer_set_surface_callback },
    { PROTOCOL_WM_POINTER_GRAB_ID , wm_pointer_grab_callback },
    { PROTOCOL_WM_POINTER_UNGRAB_ID , wm_pointer_ungrab_callback },
};
DEFINE_WM_POINTER_SERVER_PROTOCOL(wm_pointer_callbacks, 3);

static gracht_client_t* valiClient = NULL;

static void __gracht_handle_disconnect(int client)
{
    vioarr_objects_remove_by_client(client);
}

int client_initialize(void)
{
    struct gracht_client_configuration clientConfiguration;
    int                                status;

    status = gracht_link_vali_client_create(&clientConfiguration.link);
    if (status) {
        return status;
    }

    status = gracht_client_create(&clientConfiguration, &valiClient);
    if (status) {
        return status;
    }
    return status;
}

int server_initialize(int* eventIodOut)
{
    struct socket_server_configuration linkConfiguration;
    struct gracht_server_configuration serverConfiguration = { 0 };
    int                                status;
    
    gracht_os_get_server_client_address(&linkConfiguration.server_address, &linkConfiguration.server_address_length);
    gracht_os_get_server_packet_address(&linkConfiguration.dgram_address, &linkConfiguration.dgram_address_length);
    gracht_link_socket_server_create(&serverConfiguration.link, &linkConfiguration);

    // Create the set descriptor we are listening to
    serverConfiguration.set_descriptor = ioset(0);
    serverConfiguration.set_descriptor_provided = 1;
    if (serverConfiguration.set_descriptor < 0) {
        ERROR("error creating event descriptor %i", errno);
        return -1;
    }

    // Listen to client disconnects so we can remove resources
    serverConfiguration.callbacks.clientDisconnected = __gracht_handle_disconnect;
    
    status = gracht_server_initialize(&serverConfiguration);
    if (status) {
        ERROR("error initializing server library %i", errno);
        close(serverConfiguration.set_descriptor);
    }

    *eventIodOut = serverConfiguration.set_descriptor;
    return status;
}

void server_get_hid_devices(void)
{
    struct vali_link_message msg = VALI_MSG_INIT_HANDLE(GetDeviceService());
    TRACE("[server_get_hid_devices]");

    // subscribe to events from the device manager
    svc_device_subscribe(valiClient, &msg.base);

    // query all input devices
    svc_device_get_devices_by_protocol(valiClient, &msg.base, PROTOCOL_CTT_INPUT_ID);
}

void svc_device_event_protocol_device_callback(struct svc_device_protocol_device_event* input)
{
    struct vali_link_message msg = VALI_MSG_INIT_HANDLE(input->driver_id);
    TRACE("[svc_device_event_protocol_device_callback] %u", input->device_id);

    // subscribe to the driver
    ctt_input_subscribe(valiClient, &msg.base);

    // get properties of the device
    ctt_input_get_properties(valiClient, &msg.base, input->device_id);
}

void svc_device_event_device_update_callback(struct svc_device_device_update_event* input)
{
    // todo handle connection of new devices and disconnection
}

int server_run(int eventIod)
{
    struct ioset_event events[32];
    int                serverRunning = 1;
    void*              dataBuffer = malloc(GRACHT_MAX_MESSAGE_SIZE);
    int                clientIod;

    // listen to client events as well
    clientIod = gracht_client_iod(valiClient);
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
                gracht_client_wait_message(valiClient, NULL, dataBuffer, 0);
            }
            else {
                gracht_server_handle_event(events[i].data.iod, events[i].events);
            }
        }
    }

    free(dataBuffer);
    return 0;
}

/*******************************************
 * Entry Point
 *******************************************/
int main(int argc, char **argv)
{
    int eventIod;

#ifndef VIOARR_TRACEMODE
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
    gracht_client_register_protocol(valiClient, &svc_device_client_protocol);
    gracht_client_register_protocol(valiClient, &ctt_input_client_protocol);
    
    // add the server protocols we support
    gracht_server_register_protocol(&wm_core_server_protocol);
    gracht_server_register_protocol(&wm_screen_server_protocol);
    gracht_server_register_protocol(&wm_memory_server_protocol);
    gracht_server_register_protocol(&wm_memory_pool_server_protocol);
    gracht_server_register_protocol(&wm_buffer_server_protocol);
    gracht_server_register_protocol(&wm_surface_server_protocol);
    gracht_server_register_protocol(&wm_pointer_server_protocol);

    return server_run(eventIod);
}
