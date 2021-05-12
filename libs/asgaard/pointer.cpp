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
#include "include/pointer.hpp"
#include "include/surface.hpp"

#include "wm_pointer_protocol_client.h"

namespace Asgaard {
    Pointer::Pointer(uint32_t id) : Object(id)
    {

    }

    Pointer::~Pointer()
    {

    }

    void Pointer::SetSurface(const std::shared_ptr<Surface>& surface, int xOffset, int yOffset)
    {
        uint32_t id = 0;
        if (surface) {
            id = surface->Id();
        }

        // calling with an id of 0 will result in clearing the pointer surface
        wm_pointer_set_surface(APP.GrachtClient(), nullptr, Id(), id, xOffset, yOffset);
    }
}
