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
    class ObjectEvent : public Event {
    public:
        /**
         * Constructor (C++ STL string, int, int).
         *  @param errorCode Error number
         *  @param description The error message
         */
        ObjectEvent(const size_t handle, const enum wm_object_type type) 
        : Event(Event::Type::CREATION)
        , error_number(errorCode)
        , error_message(description)
        { }

        std::string Description() const { return error_message; }
        int         Code() const { return error_number; }

    private:
        size_t      error_number;
        std::string error_message;
    };
}
