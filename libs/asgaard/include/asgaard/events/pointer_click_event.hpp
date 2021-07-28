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

#include "wm_core_service.h" // for wm_pointer_button

namespace Asgaard {
    class PointerClickEvent : public Event {
    public:
        PointerClickEvent(const uint32_t pointerId, const enum wm_pointer_button button, const bool pressed) 
        : Event(Event::Type::POINTER_CLICK)
        , m_pointerId(pointerId)
        , m_button(button)
        , m_pressed(pressed)
        { }

        uint32_t               PointerId() const { return m_pointerId; }
        enum wm_pointer_button Button() const { return m_button; }
        bool                   Pressed() const { return m_pressed; }

    private:
        uint32_t               m_pointerId;
        enum wm_pointer_button m_button;
        bool                   m_pressed;
    };
}
