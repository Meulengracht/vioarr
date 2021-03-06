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

#include <asgaard/object.hpp>
#include <asgaard/error.hpp>
#include <asgaard/events/event.hpp>
#include <asgaard/events/object_event.hpp>
#include <asgaard/events/error_event.hpp>
#include <asgaard/notifications/error_notification.hpp>
#include "wm_core_service_client.h"

using namespace Asgaard;

Object::Object(uint32_t id) : m_id(id), m_globalId(0) { }
Object::~Object() { }

void Object::ExternalEvent(const Event& event) {
    switch (event.GetType())
    {
        case Event::Type::CREATION: {
            const auto& creationEvent = static_cast<const ObjectEvent&>(event);

            // store the global id provided by the server, and then notify everyone
            // about our creation
            m_globalId = creationEvent.GlobalId();
            Notify(CreatedNotification(Id()));
        } break;

        case Event::Type::ERROR: {
            const auto& error = static_cast<const ErrorEvent&>(event);
            // @todo log this error
            Notify(ErrorNotification(Id(), error.Code(), error.Description()));
        } break;
        
        default:
            break;
    }
}

void Object::Destroy() {
    Notify(Asgaard::DestroyNotification(Id()));
};
