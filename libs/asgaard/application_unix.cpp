/* ValiOS
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
 * ValiOS - Application Framework (Asgaard)
 *  - Contains the implementation of the application framework used for building
 *    graphical applications.
 */

#include <algorithm>
#include <chrono>
#include <cstring>
#include <errno.h>
#include <type_traits>
#include <gracht/client.h>
#include "include/application.hpp"
#include "include/dispatcher.hpp"
#include "include/pointer.hpp"
#include "include/screen.hpp"
#include "include/keyboard.hpp"
#include "include/object_manager.hpp"
#include "include/window_base.hpp"
#include "include/exceptions/application_exception.h"
#include "include/utils/descriptor_listener.hpp"

#include <gracht/link/socket.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <sys/socket.h>
static const char* g_vioarrPath = "/tmp/vi-srv";
static const char* g_heimdallPath = "/tmp/hd-srv";

#include "wm_core_service_client.h"
#include "wm_screen_service_client.h"
#include "wm_surface_service_client.h"
#include "wm_buffer_service_client.h"
#include "wm_pointer_service_client.h"
#include "wm_keyboard_service_client.h"

#include "hd_core_service_client.h"

namespace
{
    void InitializeClient(const char* address, gracht_client_t** clientOut)
    {
        struct gracht_client_configuration clientConfiguration;
        struct gracht_link_socket*         link;
        int                                status;
        struct sockaddr_un                 addr = { 0 };

        addr.sun_family = AF_LOCAL;
        strncpy (addr.sun_path, address, sizeof(addr.sun_path));
        addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';

        gracht_link_socket_create(&link);
        gracht_link_socket_set_type(link, gracht_link_stream_based);
        gracht_link_socket_set_address(link, (const struct sockaddr_storage*)&addr, sizeof(struct sockaddr_un));
        gracht_link_socket_set_domain(link, AF_LOCAL);

        gracht_client_configuration_init(&clientConfiguration);
        gracht_client_configuration_set_link(&clientConfiguration, (struct gracht_link*)link);
        
        status = gracht_client_create(&clientConfiguration, clientOut);
        if (status) {
            throw Asgaard::ApplicationException("failed to initialize gracht client", status);
        }
    }
}

namespace Asgaard
{
    void Application::InitializeInternal()
    {
        // initialize the heimdall client
        InitializeClient(g_heimdallPath, &m_hClient);

        gracht_client_register_protocol(m_hClient, &hd_core_client_protocol);

        // initialize the vioarr client
        InitializeClient(g_vioarrPath, &m_vClient);
        
        gracht_client_register_protocol(m_vClient, &wm_core_client_protocol);
        gracht_client_register_protocol(m_vClient, &wm_screen_client_protocol);
        gracht_client_register_protocol(m_vClient, &wm_surface_client_protocol);
        gracht_client_register_protocol(m_vClient, &wm_buffer_client_protocol);
        gracht_client_register_protocol(m_vClient, &wm_pointer_client_protocol);
        gracht_client_register_protocol(m_vClient, &wm_keyboard_client_protocol);

        // connect the client
        auto status = gracht_client_connect(m_vClient);
        if (status) {
            throw ApplicationException("failed to connect to vioarr server", status);
        }

        // Prepare the ioset to listen to multiple events
        // handle setting
        m_ioset = GetSettingInteger(Application::Settings::ASYNC_DESCRIPTOR);
        if (m_ioset <= 0) {
            m_ioset = epoll_create1(EPOLL_CLOEXEC);
            if (m_ioset <= 0) {
                throw ApplicationException("failed to initialize the ioset descriptor", errno);
            }
        }

        // add the client as a target
        AddEventDescriptor(
            gracht_client_iod(m_vClient), 
            EPOLLIN | EPOLLRDHUP,
            std::shared_ptr<Utils::DescriptorListener>(nullptr));
    }

    int Application::Execute()
    {
        struct epoll_event events[8];

        if (!IsInitialized()) {
            Initialize();
        }
        
        unsigned int tick = -1;
        while (true) {
            int num_events = epoll_wait(m_ioset, &events[0], 8, tick); // 0 && EINTR on timeout
            for (int i = 0; i < num_events; i++) {
                if (events[i].data.fd == gracht_client_iod(m_vClient)) {
                    gracht_client_wait_message(m_vClient, NULL, 0);
                }
                else {
                    auto listener = m_listeners.find(events[i].data.fd);
                    if (listener != m_listeners.end()) {
                        listener->second->DescriptorEvent(events[i].data.fd, events[i].events);
                    }
                    else if (m_defaultListener) {
                        m_defaultListener->DescriptorEvent(events[i].data.fd, events[i].events);
                    }
                }
            }

            tick = DIS.Tick();
            if (tick == 0) {
                // adjust to -1 as that means infinite in epoll
                tick = -1;
            }
        }
        
        return 0;
    }
    
    void Application::AddEventDescriptor(int iod, unsigned int events, const std::shared_ptr<Utils::DescriptorListener>& listener)
    {
        struct epoll_event evt;

        if (m_ioset == -1) {
            throw ApplicationException("Initialize() must be called before AddEventDescriptor", -1);
        }

        evt.data.fd = iod;
        evt.events = events;

        int status = epoll_ctl(m_ioset, EPOLL_CTL_ADD, iod, &evt);
        if (status) {
            throw ApplicationException("ioset_ctrl failed to add event descriptor", status);
        }

        m_listeners[iod] = listener;
    }

    void Application::DestroyInternal()
    {
        if (m_ioset != -1) {
            auto providedDescriptor = GetSettingInteger(Application::Settings::ASYNC_DESCRIPTOR);
            if (providedDescriptor <= 0) {
                // destroy if we are the owner
                close(m_ioset);
            }
        }
    }
}
