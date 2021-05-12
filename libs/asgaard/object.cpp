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

#include "include/object.hpp"
#include "include/error.hpp"
#include "wm_core_protocol_client.h"

using namespace Asgaard;

Object::Object(uint32_t id) : m_id(id) { }
Object::~Object() { }

void Object::ExternalEvent(enum ObjectEvent event, void* data) {
    switch (event)
    {
        case ObjectEvent::CREATION: {
            Notify(static_cast<int>(Notification::CREATED));
        } break;

        case ObjectEvent::ERROR: {
            struct wm_core_error_event* error = (struct wm_core_error_event*)data;
            Error appError(std::string(error->error_description), error->error_id);
            Notify(static_cast<int>(Notification::ERROR), &appError);
        } break;
        
        default:
            break;
    }
}
