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

#include <list>
#include "subscriber.hpp"

namespace Asgaard {
    namespace Utils {
        class Publisher {
        public:
            Publisher() : m_notifyActive(true) { }
            virtual ~Publisher() { }
            
            void Subscribe(Subscriber* subscriber) {
                m_subscribers.push_back(subscriber);
            }
            void Unsubscribe(Subscriber* subscriber) {
                m_subscribers.remove(subscriber);
            }
            
            void Notify(int event = 0, void* data = 0) {
                for (auto subscriber : m_subscribers) {
                    subscriber->Notification(this, event, data);
                }
            }
            
            void SetNotifyState(bool enable) { m_notifyActive = enable; }
            
        private:
            std::list<Subscriber*> m_subscribers;
            bool                   m_notifyActive;
        };
    }
}
