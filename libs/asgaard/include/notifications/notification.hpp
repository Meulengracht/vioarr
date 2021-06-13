/* ValiOS
 *
 * Copyright 2021, Philip Meulengracht
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

namespace Asgaard {
    enum class NotificationType : int {
        CREATED,
        ERROR,
        DESTROY,
        REFRESHED,
        DRAG_INITIATED,
        MINIMIZE,
        MAXIMIZE,
        CLICKED,
        TEXT_CHANGED,
        FOCUS_EVENT,
        FOCUS,

        CUSTOM_START
    };

    class Notification {
    public:
        Notification(uint32_t sourceObjectId, NotificationType type) 
            : m_type(type)
            , m_objectId(sourceObjectId)
            { }
        virtual ~Notification() { }

        const NotificationType GetType() const { return m_type; }
        const uint32_t         GetObjectId() const { return m_objectId; }

    private:
        NotificationType m_type;
        uint32_t         m_objectId;
    };

    template<NotificationType NtType>
    class NotificationTemplate : public Notification {
    public:
        NotificationTemplate(uint32_t sourceObjectId)
            : Notification(sourceObjectId, NtType)
        {}
    };

    // typename those notifications that do not require parameters
    using CreatedNotification = NotificationTemplate<NotificationType::CREATED>;
    using DestroyNotification = NotificationTemplate<NotificationType::DESTROY>;
    using RefreshedNotification = NotificationTemplate<NotificationType::REFRESHED>;
    using MinimizeNotification = NotificationTemplate<NotificationType::MINIMIZE>;
    using MaximizeNotification = NotificationTemplate<NotificationType::MAXIMIZE>;
    using ClickedNotification = NotificationTemplate<NotificationType::CLICKED>;
}
