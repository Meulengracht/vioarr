/* ValiOS
 *
 * Copyright 2020, Philip Meulengracht
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

#include "../include/widgets/icon.hpp"
#include "../include/memory_pool.hpp"
#include "../include/memory_buffer.hpp"
#include "../include/drawing/image.hpp"
#include "../include/drawing/painter.hpp"

namespace Asgaard {
    namespace Widgets {
        Icon::Icon(uint32_t id, const std::shared_ptr<Screen>& screen, const Surface* parent, const Rectangle& dimensions)
            : SubSurface(id, screen, parent, dimensions)
            , m_memory(nullptr)
            , m_currentState(IconState::NORMAL)
        {
            for (int i = 0; i < static_cast<int>(IconState::COUNT); i++) {
                m_stateAvailabilityMap[i] = false;
            }

            // Configure the surface
            SetTransparency(true);
            MarkInputRegion(Dimensions());
        }
    
        Icon::~Icon()
        {
            Destroy();
        }

        void Icon::Destroy()
        {
            for (int i = 0; i < static_cast<int>(IconState::COUNT); i++) {
                if (m_buffers[i]) {
                    m_buffers[i]->Unsubscribe(this);
                }
            }

            if (m_memory) { m_memory->Unsubscribe(this); }

            // invoke base destroy as well
            SubSurface::Destroy();
        }

        void Icon::SetImage(const Drawing::Image& image)
        {
            // clear current states
            for (int i = 0; i < static_cast<int>(IconState::COUNT); i++) {
                m_stateAvailabilityMap[i] = false;
            }

            SetStateImage(IconState::NORMAL, image);
        }

        void Icon::SetStateImage(IconState state, const Drawing::Image& image)
        {
            auto requiredPoolSize = image.Stride() * image.Height() * static_cast<int>(IconState::COUNT);
            if (!m_memory || requiredPoolSize > m_memory->Size()) {
                if (m_memory) {
                    // migrate, todo
                }
                else {
                    // Create a new pool that can hold an  of the requested size and full-ciconolor
                    m_memory = MemoryPool::Create(this, requiredPoolSize);
                }
            }

            // Create the neccessary buffer in case its first time we refer to this state
            if (!m_buffers[static_cast<int>(state)]) {
                auto memoryOffset = image.Stride() * image.Height() * static_cast<int>(state);
                m_buffers[static_cast<int>(state)] = MemoryBuffer::Create(this, m_memory, memoryOffset,
                    image.Width(), image.Height(), image.Format(), MemoryBuffer::Flags::NONE);
            }

            // Copy data from the image to the buffer
            Drawing::Painter painter(m_buffers[static_cast<int>(state)]);
            painter.RenderImage(image);

            // When the state image has been updated for the current state we should update
            // the drawn buffer. But make sure the availability map is updated
            m_stateAvailabilityMap[static_cast<int>(state)] = true;
            if (m_currentState == state) {
                SetState(state);
            }
        }
    
        void Icon::SetState(IconState state)
        {
            if (!m_buffers[static_cast<int>(state)] || 
                !m_stateAvailabilityMap[static_cast<int>(state)]) {
                return;
            }
            m_currentState = state;
    
            SetBuffer(m_buffers[static_cast<int>(state)]);
            MarkDamaged(Dimensions());
            ApplyChanges();
        }
    
        void Icon::Notification(Publisher* source, int event, void* data)
        {
            auto object = dynamic_cast<Object*>(source);
            if (object != nullptr) {
                switch (event) {
                    case static_cast<int>(Object::Notification::ERROR): {
                        Notify(static_cast<int>(Object::Notification::ERROR), data);
                    } break;
                }
                return;
            }
        }

        void Icon::OnMouseEnter(const std::shared_ptr<Pointer>&, int localX, int localY)
        {
            SetState(IconState::HOVERING);
        }

        void Icon::OnMouseLeave(const std::shared_ptr<Pointer>&)
        {
            SetState(IconState::NORMAL);
        }

        void Icon::OnMouseClick(const std::shared_ptr<Pointer>&, enum Pointer::Buttons button, bool pressed)
        {
            if (button == Pointer::Buttons::LEFT) {
                if (pressed) {
                    SetState(IconState::ACTIVE);
                    Notify(static_cast<int>(Notification::CLICKED));
                }
                else {
                    SetState(IconState::NORMAL);
                }
            }
        }
    }
}
