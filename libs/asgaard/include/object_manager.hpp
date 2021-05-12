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

#include <cstdint>
#include <map>
#include <memory>

#include "config.hpp"
#include "object.hpp"

namespace Asgaard {
    
    class ObjectManager final : Utils::Subscriber {
    public:
        ObjectManager();
        ~ObjectManager();

        template<class T>
        std::shared_ptr<T> CreateClientObject()
        {
            if (!std::is_base_of<Object, T>::value) {
                return nullptr;
            }
            
            auto object = std::make_shared<T>(CreateObjectId());
            if (object == nullptr) {
                return nullptr;
            }
            m_objects[object->Id()] = object;

            // subscribe to events so we can react on destroy invokes
            object->Subscribe(this);
            return object;
        }
        
        template<class T, typename... Params>
        std::shared_ptr<T> CreateClientObject(Params... parameters)
        {
            if (!std::is_base_of<Object, T>::value) {
                return nullptr;
            }
            
            std::shared_ptr<T> object = std::make_shared<T>(
                CreateObjectId(), std::forward<Params>(parameters)...);
            if (object == nullptr) {
                return nullptr;
            }
            m_objects[object->Id()] = object;

            // subscribe to events so we can react on destroy invokes
            object->Subscribe(this);
            return object;
        }
        
        template<class T>
        std::shared_ptr<T> CreateServerObject(uint32_t id)
        {
            if (!std::is_base_of<Object, T>::value) {
                return nullptr;
            }
            
            std::shared_ptr<T> object = std::make_shared<T>(id);
            m_objects[id] = object;
            return object;
        }
        
        template<class T, typename... Params>
        std::shared_ptr<T> CreateServerObject(uint32_t id, Params... parameters)
        {
            if (!std::is_base_of<Object, T>::value) {
                return nullptr;
            }
            
            std::shared_ptr<T> object = std::make_shared<T>(id, std::forward<Params>(parameters)...);
            m_objects[id] = object;
            return object;
        }

        ASGAARD_API std::shared_ptr<Object> operator[](uint32_t);

    public:
        static void *operator new   (size_t)   = delete;
        static void *operator new[] (size_t)   = delete;
        static void  operator delete(void*)    = delete;
        static void  operator delete[] (void*) = delete;

    private:
        void                 Notification(Utils::Publisher* source, int event, void* data) override;
        ASGAARD_API uint32_t CreateObjectId();

    private:
        std::map<uint32_t, std::shared_ptr<Object>> m_objects;
        uint32_t                                    m_idIndex;
    };
    
    extern ASGAARD_API ObjectManager OM;
}
