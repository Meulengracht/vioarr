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
#include "include/object_manager.hpp"
#include "include/window_base.hpp"
#include "include/exceptions/application_exception.h"
#include "include/utils/descriptor_listener.hpp"

// include events
#include "include/events/error_event.hpp"
#include "include/events/object_event.hpp"
#include "include/events/screen_properties_event.hpp"
#include "include/events/screen_mode_event.hpp"
#include "include/events/surface_format_event.hpp"
#include "include/events/surface_resize_event.hpp"
#include "include/events/surface_focus_event.hpp"
#include "include/events/pointer_enter_event.hpp"
#include "include/events/pointer_leave_event.hpp"
#include "include/events/pointer_move_event.hpp"
#include "include/events/pointer_click_event.hpp"
#include "include/events/pointer_scroll_event.hpp"
#include "include/events/key_event.hpp"

#ifdef MOLLENOS
#include <inet/socket.h>
#include <inet/local.h>
#include <ioset.h>
#elif defined(_WIN32)
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

#include "wm_core_service_client.h"
#include "wm_screen_service_client.h"
#include "wm_surface_service_client.h"
#include "wm_buffer_service_client.h"
#include "wm_pointer_service_client.h"
#include "wm_keyboard_service_client.h"

namespace Asgaard {
    Application APP;
    
    Application::Application() 
        : Object(0)
        , m_client(nullptr)
        , m_ioset(-1)
        , m_syncRecieved(false)
        , m_screenFound(false)
    {
        
    }

    Application::~Application()
    {
        Destroy();
    }

#ifdef MOLLENOS
    void Application::Initialize()
    {
        struct gracht_client_configuration clientConfiguration;
        struct gracht_link*                link;
        int                                status;

        if (IsInitialized()) {
            throw ApplicationException("Initialize has been called twice", -1);
        }
        
        struct sockaddr_un addr = { 0 };
        
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

        // kick off a chain reaction by asking for all objects
        wm_core_get_objects(m_client, nullptr);
        wm_core_sync(m_client, nullptr, 0);

        // wait for initialization to complete
        while (!IsInitialized()) {
            (void)gracht_client_wait_message(m_client, NULL, 0);
        }
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
#elif defined(_WIN32)
    void Application::Initialize()
    {
        struct gracht_client_configuration clientConfiguration;
        struct gracht_link_socket*         link;
        struct sockaddr_in                 addr = { 0 };
        int                                status;

        if (IsInitialized()) {
            throw ApplicationException("Initialize has been called twice", -1);
        }
        
        // initialize the WSA library
        gracht_link_socket_setup();

        // AF_INET is the Internet address family.
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(g_ipAddress);
        addr.sin_port = htons(g_portNo);

        gracht_link_socket_set_type(link, gracht_link_stream_based);
        gracht_link_socket_set_address(link, (const struct sockaddr_storage*)&addr, sizeof(struct sockaddr_in));

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

        // kick off a chain reaction by asking for all objects
        wm_core_get_objects(m_client, nullptr);
        wm_core_sync(m_client, nullptr, 0);

        // wait for initialization to complete
        while (!IsInitialized()) {
            (void)gracht_client_wait_message(m_client, NULL, 0);
        }
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
#else
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
        m_ioset = epoll_create1(EPOLL_CLOEXEC);
        if (m_ioset <= 0) {
            throw ApplicationException("failed to initialize the ioset descriptor", errno);
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
            int num_events = epoll_wait(m_ioset, &events[0], 8, -1);
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
#endif

    void Application::Destroy()
    {
        // unsubscribe from screens
        for (const auto& screen : m_screens) {
            screen->Unsubscribe(this);
        }

        m_listeners.clear();
        m_screens.clear();
        
        if (m_client != nullptr) {
            gracht_client_shutdown(m_client);
        }
    }

    bool Application::IsInitialized() const
    {
        return m_screenFound && m_syncRecieved;
    }
    
    void Application::PumpMessages()
    {
        int status = 0;
        
        if (!IsInitialized()) {
            Initialize();
        }

        while (!status) {
            status = gracht_client_wait_message(m_client, NULL, 0);
        }
    }

    void Application::ExternalEvent(const Event& event)
    {
        switch (event.GetType()) {
            case Event::Type::CREATION: {
                const auto& object = static_cast<const ObjectEvent&>(event);

                // Handle new server objects
                switch (object.ObjectType()) {
                    case WM_OBJECT_TYPE_SCREEN: {
                        auto screen = Asgaard::OM.CreateServerObject<Asgaard::Screen>(object.ObjectId());
                        screen->Subscribe(this);
                        m_screens.push_back(screen);
                    } break;

                    case WM_OBJECT_TYPE_POINTER: {
                        auto pointer = Asgaard::OM.CreateServerObject<Asgaard::Pointer>(object.ObjectId());
                    } break;
                    
                    default:
                        break;
                }
                
            }
            case Event::Type::ERROR: {
                Object::ExternalEvent(event);
            } break;
            case Event::Type::SYNC: {
                m_syncRecieved = true;
            } break;
            
            default:
                break;
        }
    }

    void Application::Notification(Publisher* source, int event, void* data)
    {
        auto screen = dynamic_cast<Screen*>(source);
        if (screen == nullptr) {
            return;
        }

        if (event == static_cast<int>(Object::Notification::CREATED)) {
            m_screenFound = true;
        }

        if (event == static_cast<int>(Object::Notification::DESTROY)) {
            std::remove_if(m_screens.begin(), m_screens.end(), 
                [screen](const std::shared_ptr<Screen>& i) { return i->Id() == screen->Id(); });
        }
    }
}

// Protocol callbacks
extern "C"
{
    void dllmain(int reason) {
        (void)reason;
    }

    // CORE PROTOCOL EVENTS
    void wm_core_event_sync_invocation(gracht_client_t* client, const uint32_t serial)
    {
        if (serial != 0) {
            auto object = Asgaard::OM[serial];
            if (!object) {
                return;
            }
            
            // publish to object
            object->ExternalEvent(Asgaard::Event(Asgaard::Event::Type::SYNC));
        }
        else {
            Asgaard::APP.ExternalEvent(Asgaard::Event(Asgaard::Event::Type::SYNC));
        }
    }

    void wm_core_event_error_invocation(gracht_client_t* client, const uint32_t id, const int errorCode, const char* description)
    {
        auto object = Asgaard::OM[id];
        if (!object) {
            // Global error, this must be handled on application level
            Asgaard::APP.ExternalEvent(Asgaard::ErrorEvent(errorCode, description));
            return;
        }
        
        // publish error to object
        object->ExternalEvent(Asgaard::ErrorEvent(errorCode, description));
    }
    
    void wm_core_event_object_invocation(gracht_client_t* client, const uint32_t id, const size_t handle, const enum wm_object_type type)
    {
        switch (type) {
            // Handle new server objects
            case WM_OBJECT_TYPE_SCREEN: {
                Asgaard::APP.ExternalEvent(Asgaard::ObjectEvent(id, handle, type));
            } break;
            case WM_OBJECT_TYPE_POINTER: {
                Asgaard::APP.ExternalEvent(Asgaard::ObjectEvent(id, handle, type));
            } break;
            
            // Handle client completion objects
            default: {
                auto object = Asgaard::OM[id];
                if (object == nullptr) {
                    // log
                    return;
                }
                object->ExternalEvent(Asgaard::ObjectEvent(id, handle, type));
            } break;
        }
    }
    
    // SCREEN PROTOCOL EVENTS
    void wm_screen_event_properties_invocation(gracht_client_t* client, const uint32_t id, const int x, const int y, const enum wm_transform transform, const int scale)
    {
        auto object = Asgaard::OM[id];
        if (!object) {
            // log
            return;
        }
        
        // publish to object
        object->ExternalEvent(Asgaard::ScreenPropertiesEvent(x, y, transform, scale));
    }
    
    void wm_screen_event_mode_invocation(gracht_client_t* client, const uint32_t id, const enum wm_mode_attributes attributes, const int resolutionX, const int resolutionY, const int refreshRate)
    {
        auto object = Asgaard::OM[id];
        if (!object) {
            // log
            return;
        }
        
        object->ExternalEvent(Asgaard::ScreenModeEvent(attributes, resolutionX, resolutionY, refreshRate));
    }
    
    // SURFACE PROTOCOL EVENTS
    void wm_surface_event_format_invocation(gracht_client_t* client, const uint32_t id, const enum wm_pixel_format format)
    {
        auto object = Asgaard::OM[id];
        if (!object) {
            // log
            return;
        }
        
        object->ExternalEvent(Asgaard::SurfaceFormatEvent(format));
    }
    
    void wm_surface_event_frame_invocation(gracht_client_t* client, const uint32_t id)
    {
        auto object = Asgaard::OM[id];
        if (!object) {
            // log
            return;
        }
        
        object->ExternalEvent(Asgaard::Event(Asgaard::Event::Type::SURFACE_FRAME));
    }
    
    void wm_surface_event_resize_invocation(gracht_client_t* client, const uint32_t id, const int width, const int height, const enum wm_surface_edge edges)
    {
        auto object = Asgaard::OM[id];
        if (!object) {
            // log
            return;
        }
        
        object->ExternalEvent(Asgaard::SurfaceResizeEvent(width, height, edges));
    }

    void wm_surface_event_focus_invocation(gracht_client_t* client, const uint32_t id, const uint8_t focus)
    {
        auto object = Asgaard::OM[id];
        if (!object) {
            // log
            return;
        }
        
        object->ExternalEvent(Asgaard::SurfaceFocusEvent(static_cast<bool>(focus)));
    }
    
    // BUFFER PROTOCOL EVENTS
    void wm_buffer_event_release_invocation(gracht_client_t* client, const uint32_t id)
    {
        auto object = Asgaard::OM[id];
        if (!object) {
            // log
            return;
        }
        
        object->ExternalEvent(Asgaard::Event(Asgaard::Event::Type::BUFFER_RELEASE));
    }

    // POINTER PROTOCOL EVENTS
    void wm_pointer_event_enter_invocation(gracht_client_t* client, const uint32_t pointerId, const uint32_t surfaceId, const int surfaceX, const int surfaceY)
    {
        auto object = Asgaard::OM[surfaceId];
        if (!object) {
            return;
        }
        
        object->ExternalEvent(Asgaard::PointerEnterEvent(pointerId, surfaceX, surfaceY));
    }

    void wm_pointer_event_leave_invocation(gracht_client_t* client, const uint32_t pointerId, const uint32_t surfaceId)
    {
        auto object = Asgaard::OM[surfaceId];
        if (!object) {
            return;
        }
        
        object->ExternalEvent(Asgaard::PointerLeaveEvent(pointerId));
    }

    void wm_pointer_event_move_invocation(gracht_client_t* client, const uint32_t pointerId, const uint32_t surfaceId, const int surfaceX, const int surfaceY)
    {
        auto object = Asgaard::OM[surfaceId];
        if (!object) {
            // log
            return;
        }
        
        object->ExternalEvent(Asgaard::PointerMoveEvent(pointerId, surfaceX, surfaceY));
    }

    void wm_pointer_event_click_invocation(gracht_client_t* client, const uint32_t pointerId, const uint32_t surfaceId, const enum wm_pointer_button button, const uint8_t pressed)
    {
        auto object = Asgaard::OM[surfaceId];
        if (!object) {
            return;
        }
        
        object->ExternalEvent(Asgaard::PointerClickEvent(pointerId, button, static_cast<bool>(pressed)));
    }

    void wm_pointer_event_scroll_invocation(gracht_client_t* client, const uint32_t pointerId, const uint32_t surfaceId, const int horz, const int vert)
    {
        auto object = Asgaard::OM[surfaceId];
        if (!object) {
            // log
            return;
        }
        
        object->ExternalEvent(Asgaard::PointerScrollEvent(pointerId, horz, vert));
    }

    // KEYBOARD PROTOCOL EVENTS
    void wm_keyboard_event_key_invocation(gracht_client_t* client, const uint32_t surfaceId, const uint32_t keycode, const uint16_t modifiers, const uint8_t pressed)
    {
        auto object = Asgaard::OM[surfaceId];
        if (!object) {
            // log
            return;
        }

        object->ExternalEvent(Asgaard::KeyEvent(keycode, modifiers, static_cast<bool>(pressed)));
    }
}
