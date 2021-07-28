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

#include <string>
#include "event.hpp"

#include "wm_core_service.h" // for wm_mode_attributes

namespace Asgaard {
    class ScreenModeEvent : public Event {
    public:
        ScreenModeEvent(const enum wm_mode_attributes attributes, const int resolutionX, const int resolutionY, const int refreshRate) 
        : Event(Event::Type::SCREEN_MODE)
        , m_resolutionX(resolutionX)
        , m_resolutionY(resolutionY)
        , m_refreshRate(refreshRate)
        , m_attributes(attributes)
        { }

        int                     ResolutionX() const { return m_resolutionX; }
        int                     ResolutionY() const { return m_resolutionY; }
        int                     RefreshRate() const { return m_refreshRate; }
        enum wm_mode_attributes Attributes() const { return m_attributes; }

    private:
        int                     m_resolutionX;
        int                     m_resolutionY;
        int                     m_refreshRate;
        enum wm_mode_attributes m_attributes;
    };
}
