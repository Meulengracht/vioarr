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

#include "wm_core_service.h" // for wm_transform

namespace Asgaard {
    class ScreenPropertiesEvent : public Event {
    public:
        ScreenPropertiesEvent(const int x, const int y, const enum wm_transform transform, const int scale) 
        : Event(Event::Type::SCREEN_PROPERTIES)
        , m_x(x)
        , m_y(y)
        , m_transform(transform)
        , m_scale(scale)
        { }

        int               X() const { return m_x; }
        int               Y() const { return m_y; }
        enum wm_transform Transform() const { return m_transform; }
        int               Scale() const { return m_scale; }

    private:
        int               m_x;
        int               m_y;
        enum wm_transform m_transform;
        int               m_scale;
    };
}
