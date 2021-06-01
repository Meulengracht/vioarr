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
#pragma once

#include "config.hpp"
#include "subsurface.hpp"

namespace Asgaard {
    class MemoryPool;
    class MemoryBuffer;
    class Pointer;

    class WindowEdge : public SubSurface {
    public:
        enum class Notification : int {
            INITIATE_DRAG = static_cast<int>(Object::Notification::CUSTOM_START)
        };

    public:
        WindowEdge(uint32_t id, 
            const std::shared_ptr<Screen>& screen, 
            const Surface* parent, 
            const Rectangle&);
        ~WindowEdge();

        void SetVisible(bool visible);
        void Destroy() override;

    private:
        void InitializeBuffer();

        void Notification(Publisher*, int = 0, void* = 0) override;
        void OnMouseEnter(const std::shared_ptr<Pointer>&, int localX, int localY) override;
        void OnMouseMove(const std::shared_ptr<Pointer>&, int localX, int localY) override;
        void OnMouseLeave(const std::shared_ptr<Pointer>&) override;
        void OnMouseClick(const std::shared_ptr<Pointer>&, enum Pointer::Buttons button, bool pressed) override;

    private:
        std::shared_ptr<Asgaard::MemoryPool>   m_memory;
        std::shared_ptr<Asgaard::MemoryBuffer> m_buffer;

        bool m_lmbHold;
    };
}
