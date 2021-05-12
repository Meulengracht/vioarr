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

#include "include/drawing/painter.hpp"
#include "include/drawing/color.hpp"
#include "include/window_title.hpp"
#include "include/memory_pool.hpp"
#include "include/memory_buffer.hpp"
#include "include/pointer.hpp"

namespace Asgaard {
    WindowTitle::WindowTitle(
            uint32_t id, 
            const std::shared_ptr<Screen>& screen, 
            const Surface* parent, 
            const Rectangle& dimensions)
        : Label(id, screen, parent, dimensions)
    {
        // we are essentially a label that is input reactive
        MarkInputRegion(Dimensions());
    }

    WindowTitle::~WindowTitle()
    {
        Destroy();
    }

    void WindowTitle::Notification(Publisher* source, int event, void* data)
    {
        auto object = dynamic_cast<Object*>(source);
        if (object) {
            switch (event) {
                case static_cast<int>(Object::Notification::ERROR): {
                    Notify(static_cast<int>(Object::Notification::ERROR));
                    return;
                } break;
            }
        }

        Widgets::Label::Notification(source, event, data);
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
            if (!m_lmbHold) {
                m_dragInOperation = false;
            }
        }
    }

    void WindowTitle::OnMouseMove(const std::shared_ptr<Pointer>& pointer, int localX, int localY)
    {
        if (m_lmbHold && !m_dragInOperation) {
            m_dragInOperation = true;
            Notify(static_cast<int>(Notification::INITIATE_DRAG), 
                reinterpret_cast<void*>(static_cast<intptr_t>(pointer->Id())));
        }
    }
}
