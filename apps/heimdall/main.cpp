/* MollenOS
 *
 * Copyright 2018, Philip Meulengracht
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
 * Terminal Implementation (Alumnious)
 * - The terminal emulator implementation for Vali. Built on manual rendering and
 *   using freetype as the font renderer.
 */

#include "heimdall.hpp"
#include <gracht/server.h>
#include <limits>

#ifdef MOLLENOS
#include <ddk/service.h>
#include <ddk/utils.h>
#include <gracht/link/socket.h>
#include <inet/local.h>
#include <io.h>
#include <ioset.h>
#include <os/process.h>
static const char* g_serverPath = "/tmp/hd-srv";
#elif defined(_WIN32)
#include <gracht/link/socket.h>
#include <windows.h>
static const char* g_ipAddress = "127.0.0.1";
static uint16_t    g_portNo    = 55556;
#else
#include <gracht/link/socket.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <sys/socket.h>
static const char* g_serverPath = "/tmp/hd-srv";
#endif

#include "hd_core_service_server.h"

static gracht_server_t*           g_valiServer = nullptr;
static struct gracht_link*        g_serverLink = nullptr;
static std::weak_ptr<HeimdallApp> g_heimdall;

static void __gracht_handle_disconnect(gracht_conn_t client)
{
    // notify heimdall
    if (auto hd = g_heimdall.lock()) {
        hd->OnApplicationUnregister(client, std::numeric_limits<unsigned int>::max());
    }
}

#ifdef MOLLENOS
static int __create_platform_link(void)
{
    struct gracht_link_socket* link;
    struct sockaddr_lc         addr = { 0 };

    addr.slc_family = AF_LOCAL;
    addr.slc_len = sizeof(addr);
    strncpy (addr.slc_addr, g_serverPath, sizeof(addr.slc_addr));

    gracht_link_socket_create(&link);
    gracht_link_socket_set_type(link, gracht_link_stream_based);
    gracht_link_socket_set_address(link, (const struct sockaddr_storage*)&addr, sizeof(struct sockaddr_lc));
    gracht_link_socket_set_domain(link, AF_LOCAL);
    gracht_link_socket_set_listen(link, 1);

    g_serverLink = reinterpret_cast<struct gracht_link*>(link);
    return gracht_server_add_link(g_valiServer, g_serverLink);
}

int server_initialize(void)
{
    struct gracht_server_configuration config;
    int                                status;
    int                                fd;
    
    gracht_server_configuration_init(&config);

    // Create the set descriptor we are listening to
    fd = ioset(0);
    if (fd < 0) {
        // trace this @todo
        return -1;
    }

    gracht_server_configuration_set_aio_descriptor(&config, fd);

    // Listen to client disconnects so we can remove resources
    config.callbacks.clientDisconnected = __gracht_handle_disconnect;
    
    status = gracht_server_create(&config, &g_valiServer);
    if (status) {
        // trace this @todo
        close(fd);
        return -1;
    }

    // create the platform link
    status = __create_platform_link();
    if (status) {
        // trace this @todo
        close(fd);
        return -1;
    }
    return fd;
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
    
    g_serverLink = static_cast<struct gracht_link*>(link);
    return gracht_server_add_link(g_valiServer, g_serverLink);
}

int server_initialize(int* eventIodOut)
{
    struct gracht_server_configuration config;
    int                                status;
    
    gracht_server_configuration_init(&config);




    // Listen to client disconnects so we can remove resources
    config.callbacks.clientDisconnected = __gracht_handle_disconnect;
    
    status = gracht_server_create(&config, &g_valiServer);
    if (status) {
        vioarr_utils_error(VISTR("error initializing server library %i"), errno);
        close(config.set_descriptor);
    }

    // create the platform link
    status = __create_platform_link();
    if (status) {
        vioarr_utils_error(VISTR("error initializing server link %i"), errno);
        close(config.set_descriptor);
    }

    *eventIodOut = config.set_descriptor;
    return status;
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

    g_serverLink = reinterpret_cast<struct gracht_link*>(link);
    return gracht_server_add_link(g_valiServer, g_serverLink);
}

int server_initialize(void)
{
    struct gracht_server_configuration config;
    int                                status;
    int                                fd;
    
    gracht_server_configuration_init(&config);

    // Create the set descriptor we are listening to
    fd = epoll_create1(0);
    if (fd < 0) {
        // trace this @todo
        return -1;
    }

    gracht_server_configuration_set_aio_descriptor(&config, fd);

    // Listen to client disconnects so we can remove resources
    config.callbacks.clientDisconnected = __gracht_handle_disconnect;
    
    status = gracht_server_create(&config, &g_valiServer);
    if (status) {
        // trace this @todo
        close(fd);
        return -1;
    }

    // create the platform link
    status = __create_platform_link();
    if (status) {
        // trace this @todo
        close(fd);
        return -1;
    }
    return fd;
}
#endif

int main(int argc, char **argv)
{
    // initialize the server for heimdall
    auto descriptor = server_initialize();
    if (descriptor == GRACHT_HANDLE_INVALID) {
        // trace this @todo
        return -1;
    }

    // register supported protocols
    gracht_server_register_protocol(g_valiServer, &hd_core_server_protocol);

    Asgaard::APP.SetSettingInteger(Asgaard::Application::Settings::ASYNC_DESCRIPTOR, descriptor);
    Asgaard::APP.SetSettingBoolean(Asgaard::Application::Settings::HEIMDALL_VISIBLE, false);
    Asgaard::APP.Initialize();

    auto screen = Asgaard::APP.GetScreen();
    auto window = screen->CreateWindow<HeimdallApp>(
        Asgaard::Rectangle(0, 0, screen->GetCurrentWidth(), screen->GetCurrentHeight()),
        g_valiServer);
    Asgaard::APP.SetDefaultEventListener(window);
    
    // store this for callbacks from global C handlers
    g_heimdall = window;

    // We only call exit() to get out, so release ownership of window
    window.reset();
    return Asgaard::APP.Execute();
}

extern "C"
{
    void hd_core_register_app_invocation(struct gracht_message* message, const unsigned int appId, const struct hd_app_icon* icon)
    {
        if (auto hd = g_heimdall.lock()) {
            hd->OnApplicationRegister(message->client, appId, icon->poolHandle, 
                icon->size, icon->iconWidth, icon->iconHeight,
                static_cast<Asgaard::PixelFormat>(icon->format));
        }
    }

    void hd_core_unregister_app_invocation(struct gracht_message* message, const unsigned int appId)
    {
        if (auto hd = g_heimdall.lock()) {
            hd->OnApplicationUnregister(message->client, appId);
        }
    }

    void hd_core_register_surface_invocation(struct gracht_message* message, const unsigned int appId, const uint32_t surfaceId)
    {
        if (auto hd = g_heimdall.lock()) {
            hd->OnSurfaceRegister(message->client, appId, surfaceId);
        }
    }

    void hd_core_unregister_surface_invocation(struct gracht_message* message, const unsigned int appId, const uint32_t surfaceId)
    {
        if (auto hd = g_heimdall.lock()) {
            hd->OnSurfaceUnregister(message->client, appId, surfaceId);
        }
    }

    void hd_core_notification_invocation(struct gracht_message* message, const unsigned int id, const char* content, const enum hd_buttons buttons)
    {
        // not yet implemented as we need to figure out custom notifications
    }
    
    void hd_core_message_box_invocation(struct gracht_message* message, const char* title, const char* body, const enum hd_context_type type, const enum hd_buttons buttons)
    {
        // not yet implemented as we need to figure out local pop-ups vs global
    }
}
