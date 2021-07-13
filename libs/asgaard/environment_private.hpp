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
#pragma once

#include "include/config.hpp"
#include <cstdint>

namespace Asgaard
{
    namespace Environment
    {
        namespace Heimdall
        {
            /**
             * Initializes the Heimdall subsystem. If this has not been invoked then
             * none of the functionality in the Heimdall namespace will be working.
             * This is automatically controlled by Asgaard.
             */
            void Initialize();

            /**
             * Cleans up any resources allocated and communicates to Heimdall that this application
             * is shutting down.
             */
            void Destroy();

            void RegisterSurface(uint32_t globalId);
        }
    }
}
