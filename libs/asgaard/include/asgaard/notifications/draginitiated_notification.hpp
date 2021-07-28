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
#include "notification.hpp"

namespace Asgaard {
    class DragInitiatedNotification : public NotificationTemplate<NotificationType::DRAG_INITIATED> {
    public:
        DragInitiatedNotification(uint32_t sourceObjectId, uint32_t pointerId) 
        : NotificationTemplate(sourceObjectId)
        , m_pointerId(pointerId)
        { }

        uint32_t GetPointerId() const { return m_pointerId; }

    private:
        uint32_t m_pointerId;
    };
}
