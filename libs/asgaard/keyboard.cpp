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
#include "include/keyboard.hpp"
#include "include/surface.hpp"

#include "wm_keyboard_service_client.h"

using namespace Asgaard;

Keyboard::Keyboard(uint32_t id) 
    : Object(id)
{ }

Keyboard::~Keyboard()
{ }


void Keyboard::Hook(const std::shared_ptr<Surface>& surface)
{
    wm_keyboard_hook(APP.VioarrClient(), nullptr, Id(), surface->Id());
}

void Keyboard::Unhook(const std::shared_ptr<Surface>& surface)
{
    wm_keyboard_unhook(APP.VioarrClient(), nullptr, Id(), surface->Id());
}
