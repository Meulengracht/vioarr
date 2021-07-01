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

#include <inet/socket.h>
#include <inet/local.h>
#include <ioset.h>

#include "wm_core_service_client.h"
#include "wm_screen_service_client.h"
#include "wm_surface_service_client.h"
#include "wm_buffer_service_client.h"
#include "wm_pointer_service_client.h"
#include "wm_keyboard_service_client.h"

namespace Asgaard {
    void Application::InitializeInternal()
    {
        struct gracht_client_configuration clientConfiguration;
        struct gracht_link*                link;
        int                                status;
        struct sockaddr_un                 addr = { 0 };
        
        addr.sun_family = AF_LOCAL;
        strncpy (addr.sun_path, g_serverPath, sizeof(addr.sun_path));
        addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';

        gracht_link_socket_set_type(link, gracht_link_stream_based);
        gracht_link_socket_set_address(link, (const struct sockaddr_storage*)&addr, sizeof(struct sockaddr_un));
        gracht_link_socket_set_domain(link, AF_LOCAL);

        gracht_client_configuration_init(&clientConfiguration);
        gracht_client_configuration_set_link(&clientConfiguration, link);
        
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

        // Prepare the ioset to listen to multiple events
        m_ioset = ioset(0);
        if (m_ioset <= 0) {
            throw ApplicationException("failed to initialize the ioset descriptor", errno);
        }
        
        // add the client as a target
        AddEventDescriptor(
            gracht_client_iod(m_client), 
            IOSETIN | IOSETCTL | IOSETLVT,
            std::shared_ptr<Utils::DescriptorListener>(nullptr));
    }
    
    int Application::Execute()
    {
        struct ioset_event events[8];

        if (!IsInitialized()) {
            Initialize();
        }
        
        while (true) {
            int num_events = ioset_wait(m_ioset, &events[0], 8, 0);
            for (int i = 0; i < num_events; i++) {
                if (events[i].data.iod == gracht_client_iod(m_client)) {
                    gracht_client_wait_message(m_client, NULL, m_messageBuffer, 0);
                }
                else {
                    auto listener = m_listeners.find(events[i].data.iod);
                    if (listener != m_listeners.end()) {
                        listener->second->DescriptorEvent(events[i].data.iod, events[i].events);
                    }
                }
            }
        }
        
        return 0;
    }
    
    void Application::AddEventDescriptor(int iod, unsigned int events, const std::shared_ptr<Utils::DescriptorListener>& listener)
    {
        if (m_ioset == -1) {
            throw ApplicationException("Initialize() must be called before AddEventDescriptor", -1);
        }

        int status = ioset_ctrl(m_ioset, IOSET_ADD, iod,
               &(struct ioset_event) {
                    .events   = events,
                    .data.iod = iod
               });
        if (status) {
            throw ApplicationException("ioset_ctrl failed to add event descriptor", status);
        }

        m_listeners[iod] = listener;
    }

    void Application::DestroyInternal()
    {
        
    }
}
