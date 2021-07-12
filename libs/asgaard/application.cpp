/**
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
#include "environment_private.hpp"

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
        , m_defaultListener(nullptr)
        , m_vClient(nullptr)
        , m_hClient(nullptr)
        , m_ioset(ASYNC_HANDLE_INVALID)
        , m_syncRecieved(false)
        , m_screenFound(false)
    {
        // initialize default settings
        SetSettingBoolean(Settings::HEIMDALL_VISIBLE, true);
    }

    Application::~Application()
    {
        Destroy();
    }

    void Application::Destroy()
    {
        // unsubscribe from screens
        for (const auto& screen : m_screens) {
            screen->Unsubscribe(this);
        }

        m_listeners.clear();
        m_screens.clear();
        
        if (m_vClient != nullptr) {
            gracht_client_shutdown(m_vClient);
        }

        if (m_hClient != nullptr) {
            gracht_client_shutdown(m_hClient);
        }
        
        // allow os-specific cleanup
        DestroyInternal();
    }

    void Application::Initialize()
    {
        if (IsInitialized()) {
            throw ApplicationException("Initialize has been called twice", -1);
        }

        // perform per-platform initialization
        InitializeInternal();
        
        // kick off a chain reaction by asking for all objects
        wm_core_get_objects(m_vClient, nullptr);
        wm_core_sync(m_vClient, nullptr, 0);

        // wait for initialization to complete
        while (!IsInitialized()) {
            (void)gracht_client_wait_message(m_vClient, nullptr, GRACHT_MESSAGE_BLOCK);
        }

        // initialize heimdall
        auto initializeHeimdall = GetSettingBoolean(Settings::HEIMDALL_VISIBLE);
        if (initializeHeimdall) {
            Environment::Heimdall::Initialize();
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
            status = gracht_client_wait_message(m_vClient, NULL, 0);
        }
    }

    void Application::SetDefaultEventListener(const std::shared_ptr<Utils::DescriptorListener>& listener)
    {
        m_defaultListener = listener;
    }

    void Application::SetSettingPointer(Settings setting, void* value)
    {
        // this operation is only valid before setup
        if (IsInitialized()) {
            // throw?
            return;
        }

        SettingValue settingValue{};
        settingValue.data.pValue = value;

        m_settings[static_cast<int>(setting)] = settingValue;
    }

    void Application::SetSettingString(Settings setting, std::string value)
    {
        // this operation is only valid before setup
        if (IsInitialized()) {
            // throw?
            return;
        }

        SettingValue settingValue{};
        settingValue.data.strValue = value;

        m_settings[static_cast<int>(setting)] = settingValue;
    }

    void Application::SetSettingBoolean(Settings setting, bool value)
    {
        // this operation is only valid before setup
        if (IsInitialized()) {
            // throw?
            return;
        }

        SettingValue settingValue{};
        settingValue.data.bValue = value;

        m_settings[static_cast<int>(setting)] = settingValue;
    }

    void Application::SetSettingInteger(Settings setting, int value)
    {
        // this operation is only valid before setup
        if (IsInitialized()) {
            // throw?
            return;
        }

        SettingValue settingValue{};
        settingValue.data.iValue = value;

        m_settings[static_cast<int>(setting)] = settingValue;
    }

    void* Application::GetSettingPointer(Settings setting)
    {
        auto entry = m_settings.find(static_cast<int>(setting));
        if (entry == std::end(m_settings)) {
            return nullptr;
        }
        return (*entry).second.data.pValue;
    }

    std::string Application::GetSettingString(Settings setting)
    {
        auto entry = m_settings.find(static_cast<int>(setting));
        if (entry == std::end(m_settings)) {
            return std::string{};
        }
        return (*entry).second.data.strValue;
    }

    bool Application::GetSettingBoolean(Settings setting)
    {
        auto entry = m_settings.find(static_cast<int>(setting));
        if (entry == std::end(m_settings)) {
            return false;
        }
        return (*entry).second.data.bValue;
    }

    int Application::GetSettingInteger(Settings setting)
    {
        auto entry = m_settings.find(static_cast<int>(setting));
        if (entry == std::end(m_settings)) {
            return 0;
        }
        return (*entry).second.data.iValue;
    }

    std::shared_ptr<Keyboard> Application::GetKeyboard() const
    {
        auto entry = std::find_if(
            std::begin(m_inputs),
            std::end(m_inputs),
            [] (const auto& obj) {
                auto keyboard = std::dynamic_pointer_cast<Keyboard>(obj);
                return keyboard != nullptr;
            }
        );
        if (entry == std::end(m_inputs)) {
            return std::shared_ptr<Keyboard>(nullptr);
        }
        return std::dynamic_pointer_cast<Keyboard>((*entry));
    }

    std::shared_ptr<Pointer> Application::GetPointer() const
    {
        auto entry = std::find_if(
            std::begin(m_inputs),
            std::end(m_inputs),
            [] (const auto& obj) {
                auto keyboard = std::dynamic_pointer_cast<Pointer>(obj);
                return keyboard != nullptr;
            }
        );
        if (entry == std::end(m_inputs)) {
            return std::shared_ptr<Pointer>(nullptr);
        }
        return std::dynamic_pointer_cast<Pointer>(*entry);
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
                        m_inputs.push_back(pointer);
                    } break;

                    case WM_OBJECT_TYPE_KEYBOARD: {
                        auto keyboard = Asgaard::OM.CreateServerObject<Asgaard::Keyboard>(object.ObjectId());
                        m_inputs.push_back(keyboard);
                    } break;
                    
                    default:
                        break;
                }
                
            }
            case Event::Type::SYNC: {
                m_syncRecieved = true;
            } break;
            
            default:
                break;
        }

        // always call base-handler for these types
        Object::ExternalEvent(event);
    }

    void Application::Notification(const Publisher* source, const Asgaard::Notification& notification)
    {
        const auto screen = dynamic_cast<const Screen*>(source);
        if (screen == nullptr) {
            return;
        }

        if (notification.GetType() == NotificationType::CREATED) {
            m_screenFound = true;
        }

        if (notification.GetType() == NotificationType::DESTROY) {
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
    
    void wm_core_event_object_invocation(gracht_client_t* client, const uint32_t id, 
        const uint32_t gid, const size_t handle, const enum wm_object_type type)
    {
        switch (type) {
            // Handle new server objects
            case WM_OBJECT_TYPE_SCREEN:
            case WM_OBJECT_TYPE_KEYBOARD:
            case WM_OBJECT_TYPE_POINTER: {
                // server objects do not have seperate global identifiers
                Asgaard::APP.ExternalEvent(Asgaard::ObjectEvent(id, handle, type));
            } break;
            
            // Handle client completion objects
            default: {
                auto object = Asgaard::OM[id];
                if (object == nullptr) {
                    // log
                    return;
                }
                object->ExternalEvent(Asgaard::ObjectEvent(id, gid, handle, type));
            } break;
        }
    }
    
    void wm_core_event_destroy_invocation(gracht_client_t* client, const uint32_t id)
    {
        auto object = Asgaard::OM[id];
        if (object) {
            object->Destroy();
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
