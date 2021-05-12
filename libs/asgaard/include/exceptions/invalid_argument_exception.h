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

#include <exception>
#include <string>

namespace Asgaard {
    class InvalidArgumentException : virtual public std::exception {
    protected:
        std::string error_message;
        
    public:
        /**
         * Constructor (C++ STL string, int, int).
         *  @param msg The error message
         *  @param err_num Error number
         */
        explicit InvalidArgumentException(const std::string& msg) :
            error_message(msg)
            { }

        /**
         * Destructor.
         *  Virtual to allow for subclassing.
         */
        virtual ~InvalidArgumentException() throw () { }

        /** 
         * Returns a pointer to the (constant) error description.
         *  @return A pointer to a const char*. The underlying memory
         *  is in possession of the Except object. Callers must
         *  not attempt to free the memory.
         */
        virtual const char* what() const throw () { return error_message.c_str(); }
    };
}
