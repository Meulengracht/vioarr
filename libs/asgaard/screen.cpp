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
#include "include/application.hpp"
#include "include/screen.hpp"
#include "include/window_base.hpp"
#include "include/events/screen_properties_event.hpp"
#include "include/events/screen_mode_event.hpp"

#include "wm_core_service_client.h"
#include "wm_screen_service_client.h"

static Asgaard::Screen::ScreenTransform ConvertProtocolTransform(enum wm_transform transform)
{
    switch (transform)
    {
        case WM_TRANSFORM_NO_TRANSFORM:
            return Asgaard::Screen::NONE;
        case WM_TRANSFORM_ROTATE_90:
            return Asgaard::Screen::ROTATE_90;
        case WM_TRANSFORM_ROTATE_180:
            return Asgaard::Screen::ROTATE_180;
        case WM_TRANSFORM_ROTATE_270:
            return Asgaard::Screen::ROTATE_270;

        default:
            return Asgaard::Screen::NONE;
    }
}

static Asgaard::Screen::ScreenMode::ModeFlags ConvertProtocolMode(enum wm_mode_attributes attributes)
{
    unsigned int screenMode = (unsigned int)Asgaard::Screen::ScreenMode::ModeFlags::NONE;
    if (attributes & WM_MODE_ATTRIBUTES_CURRENT) {
        screenMode |= (unsigned int)Asgaard::Screen::ScreenMode::ModeFlags::MODE_CURRENT;
    }
    if (attributes & WM_MODE_ATTRIBUTES_PREFERRED) {
        screenMode |= (unsigned int)Asgaard::Screen::ScreenMode::ModeFlags::MODE_PREFERRED;
    }
    return (Asgaard::Screen::ScreenMode::ModeFlags)screenMode;
}

namespace Asgaard {
    Screen::Screen(uint32_t id)
        : Object(id)
        , m_positionX(0)
        , m_positionY(0)
        , m_scale(0)
        , m_transform(ScreenTransform::NONE)
    {
        // Get properties of the screen
        wm_screen_get_properties(APP.VioarrClient(), nullptr, id);
    }

    Screen::~Screen()
    {
        Destroy();
    }

    void Screen::Destroy()
    {
        for (const auto& window : m_windows) {
            window->Unsubscribe(this);
        }

        m_modes.clear();
        m_windows.clear();

        // call base destroy as well
        Object::Destroy();
    }
    
    int Screen::GetCurrentWidth() const
    {
        auto it = std::find_if(m_modes.begin(), m_modes.end(), 
            [](const std::unique_ptr<ScreenMode>& mode) { return mode->IsCurrent(); });
        if (it != m_modes.end()) {
            return (*it)->ResolutionX();
        }
        return -1;
    }
    
    int Screen::GetCurrentHeight() const
    {
        auto it = std::find_if(m_modes.begin(), m_modes.end(), 
            [](const std::unique_ptr<ScreenMode>& mode) { return mode->IsCurrent(); });
        if (it != m_modes.end()) {
            return (*it)->ResolutionY();
        }
        return -1;
    }
    
    int Screen::GetCurrentRefreshRate() const
    {
        auto it = std::find_if(m_modes.begin(), m_modes.end(), 
            [](const std::unique_ptr<ScreenMode>& mode) { return mode->IsCurrent(); });
        if (it != m_modes.end()) {
            return (*it)->RefreshRate();
        }
        return -1;
    }

    const std::list<std::unique_ptr<Screen::ScreenMode>>& Screen::GetModes() const
    {
        return m_modes;
    }
    
    void Screen::ExternalEvent(const Event& event)
    {
        switch (event.GetType()) {
            case Event::Type::SYNC: {
                Notify(CreatedNotification(Id()));
            } break;
            
            case Event::Type::SCREEN_PROPERTIES: {
                const auto& properties = static_cast<const ScreenPropertiesEvent&>(event);
                
                // update stored information
                m_positionX   = properties.X();
                m_positionY   = properties.Y();
                m_scale       = properties.Scale();
                m_transform   = ConvertProtocolTransform(properties.Transform());
                
                // continue this charade and ask for modes, end with a sync
                wm_screen_get_modes(APP.VioarrClient(), nullptr, Id());
                wm_core_sync(APP.VioarrClient(), nullptr, Id());
            } break;
            
            case Event::Type::SCREEN_MODE: {
                const auto& mode = static_cast<const ScreenModeEvent&>(event);
                
                std::unique_ptr<ScreenMode> screenMode( 
                    new ScreenMode(ConvertProtocolMode(mode.Attributes()), mode.ResolutionX(), 
                        mode.ResolutionY(), mode.RefreshRate()));
                
                m_modes.push_back(std::move(screenMode));
            } break;
            
            default:
                break;
        }

        // always call base-handler for these types
        Object::ExternalEvent(event);
    }

    void Screen::Notification(const Publisher* source, const Asgaard::Notification& notification)
    {
        if (notification.GetType() == NotificationType::DESTROY) {
            std::remove_if(m_windows.begin(), m_windows.end(), 
                [objectId = notification.GetObjectId()](const std::shared_ptr<WindowBase>& i) { 
                    return i->Id() == objectId; 
            });
        }
        
        Object::Notification(source, notification);
    }
}
