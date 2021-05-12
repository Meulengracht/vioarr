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
#include "include/window_edge.hpp"
#include "include/memory_pool.hpp"
#include "include/memory_buffer.hpp"
#include "include/pointer.hpp"

namespace Asgaard {
    WindowEdge::WindowEdge(
            uint32_t id, 
            const std::shared_ptr<Screen>& screen, 
            const Surface* parent, 
            const Rectangle& dimensions)
        : SubSurface(id, screen, parent, dimensions)
    {
        // create required memory
        auto poolSize = (dimensions.Width() * dimensions.Height() * 4);
        m_memory = MemoryPool::Create(this, poolSize);
        m_buffer = MemoryBuffer::Create(this, m_memory, 0, 
            dimensions.Width(), dimensions.Height(), 
            PixelFormat::A8R8G8B8, MemoryBuffer::Flags::NONE);

        // clear buffer transparent
        InitializeBuffer();
    }

    WindowEdge::~WindowEdge()
    {
        Destroy();
    }

    void WindowEdge::Destroy()
    {
        if (m_memory)    { m_memory->Unsubscribe(this); }
        if (m_buffer)    { m_buffer->Unsubscribe(this); }

        // invoke base destroy
        SubSurface::Destroy();
    }

    void WindowEdge::InitializeBuffer()
    {
        Drawing::Painter paint(m_buffer);

        paint.SetFillColor(0, 0, 0, 0);
        paint.RenderFill();

        SetTransparency(true);
        MarkInputRegion(Dimensions());
    }

    void WindowEdge::SetVisible(bool visible)
    {
        if (visible) {
            SetBuffer(m_buffer);
            MarkDamaged(Dimensions());
        }
        else {
            auto nullp = std::shared_ptr<MemoryBuffer>(nullptr);
            SetBuffer(nullp);
        }
        ApplyChanges();
    }

    void WindowEdge::Notification(Publisher* source, int event, void* data)
    {
        auto object = dynamic_cast<Object*>(source);
        if (object) {
            switch (event) {
                case static_cast<int>(Object::Notification::ERROR): {
                    Notify(static_cast<int>(Object::Notification::ERROR));
                } break;
            }
        }
    }

    void WindowEdge::OnMouseEnter(const std::shared_ptr<Pointer>&, int localX, int localY)
    {
        // switch pointer surface
    }

    void WindowEdge::OnMouseLeave(const std::shared_ptr<Pointer>&)
    {
        // reset pointer surface
    }

    void WindowEdge::OnMouseClick(const std::shared_ptr<Pointer>& pointer, enum Pointer::Buttons button, bool pressed)
    {
        if (button == Pointer::Buttons::LEFT) {
            m_lmbHold = pressed;
            if (!m_lmbHold) {
                m_dragInOperation = false;
            }
        }
    }

    void WindowEdge::OnMouseMove(const std::shared_ptr<Pointer>& pointer, int localX, int localY)
    {
        if (m_lmbHold && !m_dragInOperation) {
            m_dragInOperation = true;
            Notify(static_cast<int>(Notification::INITIATE_DRAG), 
                reinterpret_cast<void*>(static_cast<intptr_t>(pointer->Id())));
        }
    }
}
