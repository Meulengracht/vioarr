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

#include "include/object_manager.hpp"
#include "include/object.hpp"
#include <gracht/client.h>
#include <type_traits>

namespace Asgaard {
    ObjectManager OM;
    
    ObjectManager::ObjectManager() :
        m_idIndex(0x100) { }

    ObjectManager::~ObjectManager()
    {
        // unsubscribe to all objects
        for (const auto& object : m_objects) {
            if (object.first > 256 && object.first < 0x80000000) {
                object.second->Unsubscribe(this);
            }
        }

        m_objects.clear();
    }
    
    std::shared_ptr<Object> ObjectManager::operator[](uint32_t index)
    {
        std::map<uint32_t, std::shared_ptr<Object>>::iterator it = m_objects.find(index);
        if (it != m_objects.end()) {
            return it->second;
        }
        return nullptr;
    }

    void ObjectManager::Notification(Utils::Publisher* source, int event, void* data)
    {
        auto object = dynamic_cast<Object*>(source);
        if (object == nullptr) {
            return;
        }

        if (event == static_cast<int>(Object::Notification::DESTROY)) {
            m_objects.erase(object->Id());
        }
    }
    
    uint32_t ObjectManager::CreateObjectId()
    {
        return m_idIndex++;
    }
}
