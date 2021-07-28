/**
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

#include <asgaard/drawing/painter.hpp>
#include <asgaard/drawing/color.hpp>
#include <asgaard/window_title.hpp>
#include <asgaard/memory_pool.hpp>
#include <asgaard/memory_buffer.hpp>
#include <asgaard/pointer.hpp>

#include <asgaard/notifications/draginitiated_notification.hpp>

namespace Asgaard {
    WindowTitle::WindowTitle(
            uint32_t id, 
            const std::shared_ptr<Screen>& screen,
            const Rectangle& dimensions)
        : Label(id, screen, dimensions)
        , m_lmbHold(false)
    {
        // we are essentially a label that is input reactive
        MarkInputRegion(Dimensions());
    }

    WindowTitle::~WindowTitle()
    {
        Destroy();
    }

    void WindowTitle::Notification(const Publisher* source, const Asgaard::Notification& notification)
    {
        switch (notification.GetType()) {
            case NotificationType::ERROR: {
                // log error
            } break;

            default:
                break;
        }

        Widgets::Label::Notification(source, notification);
    }

    void WindowTitle::OnMouseEnter(const std::shared_ptr<Pointer>&, int localX, int localY)
    {
        // switch pointer surface
    }

    void WindowTitle::OnMouseLeave(const std::shared_ptr<Pointer>&)
    {
        // reset pointer surface
    }

    void WindowTitle::OnMouseClick(const std::shared_ptr<Pointer>& pointer, enum Pointer::Buttons button, bool pressed)
    {
        if (button == Pointer::Buttons::LEFT) {
            m_lmbHold = pressed;
        }
    }

    void WindowTitle::OnMouseMove(const std::shared_ptr<Pointer>& pointer, int localX, int localY)
    {
        if (m_lmbHold) {
            Notify(DragInitiatedNotification(Id(), pointer->Id()));
            m_lmbHold = false;
        }
    }
}
