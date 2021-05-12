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

#include <cstdint>
#include "config.hpp"
#include "utils/publisher.hpp"
#include "utils/subscriber.hpp"

namespace Asgaard {
    class Object : public Utils::Publisher, public Utils::Subscriber {
    public:
        enum class ObjectEvent {
            ERROR,
            SYNC,
            CREATION,
            SCREEN_PROPERTIES,
            SCREEN_MODE,
            SURFACE_FORMAT,
            SURFACE_RESIZE,
            SURFACE_FRAME,
            SURFACE_FOCUSED,
            SURFACE_UNFOCUSED,
            BUFFER_RELEASE,
            KEY_EVENT,
            POINTER_ENTER,
            POINTER_LEAVE,
            POINTER_MOVE,
            POINTER_CLICK
        };

        enum class Notification : int {
            CREATED = 0,
            ERROR,
            DESTROY,

            CUSTOM_START
        };
        
    public:
        Object(uint32_t id);
        ASGAARD_API virtual ~Object();

    public:
        uint32_t Id() const { return m_id; }
        
        virtual void Destroy() {
            Notify(static_cast<int>(Notification::DESTROY));
        };

        virtual void ExternalEvent(enum ObjectEvent event, void* data = 0);
        
    private:
        uint32_t m_id;
    };
}
