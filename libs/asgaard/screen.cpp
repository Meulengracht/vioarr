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

#include "include/application.hpp"
#include "include/screen.hpp"
#include "include/window_base.hpp"
#include "wm_core_protocol_client.h"
#include "wm_screen_protocol_client.h"

static Asgaard::Screen::ScreenTransform ConvertProtocolTransform(enum wm_screen_transform transform)
{
    switch (transform)
    {
        case no_transform:
            return Asgaard::Screen::NONE;
        case rotate_90:
            return Asgaard::Screen::ROTATE_90;
        case rotate_180:
            return Asgaard::Screen::ROTATE_180;
        case rotate_270:
            return Asgaard::Screen::ROTATE_270;
    }
}

static Asgaard::Screen::ScreenMode::ModeFlags ConvertProtocolMode(enum wm_screen_mode_attributes attributes)
{
    unsigned int screenMode = (unsigned int)Asgaard::Screen::ScreenMode::ModeFlags::NONE;
    if (attributes & mode_current) {
        screenMode |= (unsigned int)Asgaard::Screen::ScreenMode::ModeFlags::MODE_CURRENT;
    }
    if (attributes & mode_preferred) {
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
        wm_screen_get_properties(APP.GrachtClient(), nullptr, id);
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
    
    void Screen::ExternalEvent(enum ObjectEvent event, void* data)
    {
        switch (event) {
            case Object::ObjectEvent::SYNC: {
                Notify(static_cast<int>(Notification::CREATED));
            } break;
            
            case Object::ObjectEvent::SCREEN_PROPERTIES: {
                struct wm_screen_screen_properties_event* properties = 
                    (struct wm_screen_screen_properties_event*)data;
                
                // update stored information
                m_positionX   = properties->x;
                m_positionY   = properties->y;
                m_scale       = properties->scale;
                m_transform   = ConvertProtocolTransform(properties->transform);
                
                // continue this charade and ask for modes, end with a sync
                wm_screen_get_modes(APP.GrachtClient(), nullptr, Id());
                wm_core_sync(APP.GrachtClient(), nullptr, Id());
            } break;
            
            case Object::ObjectEvent::SCREEN_MODE: {
                struct wm_screen_mode_event* mode = 
                    (struct wm_screen_mode_event*)data;
                
                std::unique_ptr<ScreenMode> screenMode( 
                    new ScreenMode(ConvertProtocolMode(mode->flags), mode->resolution_x, 
                        mode->resolution_y, mode->refresh_rate));
                
                m_modes.push_back(std::move(screenMode));
            } break;
            
            default:
                Object::ExternalEvent(event, data);
                break;
        }
    }

    void Screen::Notification(Publisher* source, int event, void* data)
    {
        auto window = dynamic_cast<WindowBase*>(source);
        if (window == nullptr) {
            return;
        }

        if (event == static_cast<int>(Object::Notification::DESTROY)) {
            std::remove_if(m_windows.begin(), m_windows.end(), 
                [window](const std::shared_ptr<WindowBase>& i) { return i->Id() == window->Id(); });
        }
    }
}
