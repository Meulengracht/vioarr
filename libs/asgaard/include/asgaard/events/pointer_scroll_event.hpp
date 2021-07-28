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

namespace Asgaard {
    class PointerScrollEvent : public Event {
    public:
        PointerScrollEvent(const uint32_t pointerId, const int horizontal, const int vertical) 
        : Event(Event::Type::POINTER_SCROLL)
        , m_pointerId(pointerId)
        , m_horizontal(horizontal)
        , m_vertical(vertical)
        { }

        uint32_t PointerId() const { return m_pointerId; }
        int      ChangeX() const { return m_horizontal; }
        int      ChangeY() const { return m_vertical; }

    private:
        uint32_t m_pointerId;
        int      m_horizontal;
        int      m_vertical;
    };
}
