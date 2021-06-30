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
#include <cstring>
#include <errno.h>
#include <type_traits>
#include <gracht/client.h>
#include "include/application.hpp"
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
static const char* g_serverPath = "/tmp/vi-srv";

#include "wm_core_service_client.h"
#include "wm_screen_service_client.h"
#include "wm_surface_service_client.h"
#include "wm_buffer_service_client.h"
#include "wm_pointer_service_client.h"
#include "wm_keyboard_service_client.h"

namespace Asgaard {
    void Application::Initialize()
    {
        struct gracht_client_configuration clientConfiguration;
        struct gracht_link_socket*         link;
        int                                status;
        struct sockaddr_un                 addr = { 0 };

        if (IsInitialized()) {
            throw ApplicationException("Initialize has been called twice", -1);
        }
        
        addr.sun_family = AF_LOCAL;
        strncpy (addr.sun_path, g_serverPath, sizeof(addr.sun_path));
        addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';

        gracht_link_socket_create(&link);
        gracht_link_socket_set_type(link, gracht_link_stream_based);
        gracht_link_socket_set_address(link, (const struct sockaddr_storage*)&addr, sizeof(struct sockaddr_un));
        gracht_link_socket_set_domain(link, AF_LOCAL);

        gracht_client_configuration_init(&clientConfiguration);
        gracht_client_configuration_set_link(&clientConfiguration, (struct gracht_link*)link);
        
        status = gracht_client_create(&clientConfiguration, &m_client);
        if (status) {
            throw ApplicationException("failed to initialize gracht client library", status);
        }
        
        gracht_client_register_protocol(m_client, &wm_core_client_protocol);
        gracht_client_register_protocol(m_client, &wm_screen_client_protocol);
        gracht_client_register_protocol(m_client, &wm_surface_client_protocol);
        gracht_client_register_protocol(m_client, &wm_buffer_client_protocol);
        gracht_client_register_protocol(m_client, &wm_pointer_client_protocol);
        gracht_client_register_protocol(m_client, &wm_keyboard_client_protocol);

        // connect the client
        status = gracht_client_connect(m_client);
        if (status) {
            throw ApplicationException("failed to connect to gracht server", status);
        }

        // Prepare the ioset to listen to multiple events
        // handle setting
        auto descriptorPointer = GetSetting(Application::Settings::ASYNC_DESCRIPTOR);
        if (descriptorPointer) {
            m_ioset = *static_cast<int*>(descriptorPointer);
            if (m_ioset <= 0) {
                throw ApplicationException("settings: invalid ioset descriptor", errno);
            }
        }
        else {
            m_ioset = epoll_create1(EPOLL_CLOEXEC);
            if (m_ioset <= 0) {
                throw ApplicationException("failed to initialize the ioset descriptor", errno);
            }
        }

        // add the client as a target
        AddEventDescriptor(
            gracht_client_iod(m_client), 
            EPOLLIN | EPOLLRDHUP,
            std::shared_ptr<Utils::DescriptorListener>(nullptr));

        // kick off a chain reaction by asking for all objects
        wm_core_get_objects(m_client, nullptr);
        wm_core_sync(m_client, nullptr, 0);

        // wait for initialization to complete
        while (!IsInitialized()) {
            (void)gracht_client_wait_message(m_client, NULL, GRACHT_MESSAGE_BLOCK);
        }
    }

    int Application::Execute()
    {
        struct epoll_event events[8];

        if (!IsInitialized()) {
            Initialize();
        }
        
        while (true) {
            int num_events = epoll_wait(m_ioset, &events[0], 8, -1); // 0 && EINTR on timeout
            for (int i = 0; i < num_events; i++) {
                if (events[i].data.fd == gracht_client_iod(m_client)) {
                    gracht_client_wait_message(m_client, NULL, 0);
                }
                else {
                    auto listener = m_listeners.find(events[i].data.fd);
                    if (listener != m_listeners.end()) {
                        listener->second->DescriptorEvent(events[i].data.fd, events[i].events);
                    }
                }
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
            auto descriptorPointer = GetSetting(Application::Settings::ASYNC_DESCRIPTOR);
            if (!descriptorPointer) {
                // destroy if we are the owner
                close(m_ioset);
            }
        }
    }
}