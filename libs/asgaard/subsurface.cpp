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

#include "include/application.hpp"
#include "include/object_manager.hpp"
#include "include/subsurface.hpp"

#include "wm_surface_protocol_client.h"

namespace Asgaard {
    SubSurface::SubSurface(uint32_t id, const std::shared_ptr<Screen>& screen, const Surface* parent, const Rectangle& dimensions)
        : Surface(id, screen, parent, dimensions) { }

    void SubSurface::Resize(int width, int height)
    {
        wm_surface_resize_subsurface(APP.GrachtClient(), nullptr, Id(), width, height);
    }

    void SubSurface::Move(int parentX, int parentY)
    {
        wm_surface_move_subsurface(APP.GrachtClient(), nullptr, Id(), parentX, parentY);
    }
}
