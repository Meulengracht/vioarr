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

#include "wm_core_service.h" // for wm_object_type

namespace Asgaard {
    class ObjectEvent : public Event {
    public:
        ObjectEvent(const uint32_t id, const uint32_t gid, const size_t handle, const enum wm_object_type type) 
        : Event(Event::Type::CREATION)
        , m_objectId(id)
        , m_objectGlobalId(gid)
        , m_objectType(type)
        , m_handle(handle)
        { }
        ObjectEvent(const uint32_t id, const size_t handle, const enum wm_object_type type) 
        : ObjectEvent(id, 0, handle, type)
        { }

        uint32_t            ObjectId() const { return m_objectId; }
        uint32_t            GlobalId() const { return m_objectGlobalId; }
        size_t              NativeHandle() const { return m_handle; }
        enum wm_object_type ObjectType() const { return m_objectType; }

    private:
        uint32_t            m_objectId;
        uint32_t            m_objectGlobalId;
        enum wm_object_type m_objectType;
        size_t              m_handle;
    };
}
