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
#pragma once

#include <cstdint>
#include <list>
#include <memory>
#include "config.hpp"
#include "object.hpp"

namespace Asgaard {
    class WindowBase;

    class Screen : public Object, public std::enable_shared_from_this<Screen> {
    public:
        enum ScreenTransform {
            NONE,
            ROTATE_90,
            ROTATE_180,
            ROTATE_270,
        };
    public:
        class ScreenMode {
        public:
            enum class ModeFlags : unsigned int {
                NONE = 0,
                MODE_CURRENT = 1,
                MODE_PREFERRED = 2
            };
            
        public:
            ScreenMode(enum ModeFlags flags, int resolutionX, int resolutionY, int refreshRate)
                : m_flags(flags), m_resolutionX(resolutionX), m_resolutionY(resolutionY), m_refreshRate(refreshRate)
                { }
                
            bool IsCurrent()   const { return ((unsigned int)m_flags & (unsigned int)Asgaard::Screen::ScreenMode::ModeFlags::MODE_CURRENT) != 0; }
            int  ResolutionX() const { return m_resolutionX; }
            int  ResolutionY() const { return m_resolutionY; }
            int  RefreshRate() const { return m_refreshRate; }
            
        private:
            ModeFlags m_flags;
            int       m_resolutionX;
            int       m_resolutionY;
            int       m_refreshRate;
        };
        
    public:
        Screen(uint32_t id);
        ASGAARD_API ~Screen();

        void Destroy() override;
        
        ASGAARD_API int GetCurrentWidth() const;
        ASGAARD_API int GetCurrentHeight() const;
        ASGAARD_API int GetCurrentRefreshRate() const;

        ASGAARD_API const std::list<std::unique_ptr<ScreenMode>>& GetModes() const;
        
    public:
        template<class WC, typename... Params>
        std::shared_ptr<WC> CreateWindow(Params... parameters) {
            if (!std::is_base_of<WindowBase, WC>::value) {
                return nullptr;
            }

            auto window = OM.CreateClientObject<WC, const std::shared_ptr<Screen>&, Params...>(
                shared_from_this(), std::forward<Params>(parameters)...);
            if (window == nullptr) {
                return nullptr;
            }

            // subscribe to the window events so we can listen for destroy
            window->Subscribe(this);
            
            m_windows.push_back(window);
            return window;
        }

        void ExternalEvent(enum ObjectEvent event, void* data = 0) override;
        void Notification(Publisher*, int = 0, void* = 0) override;
        
    private:
        std::list<std::unique_ptr<ScreenMode>> m_modes;
        std::list<std::shared_ptr<WindowBase>> m_windows;
        
        int             m_positionX;
        int             m_positionY;
        int             m_scale;
        ScreenTransform m_transform;
    };
}
