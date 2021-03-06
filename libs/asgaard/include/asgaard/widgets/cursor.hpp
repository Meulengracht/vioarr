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
 * ValiOS - Application Environment (Asgaard)
 *  - Contains the implementation of the application environment to support
 *    graphical user interactions.
 */
#pragma once

#include "../memory_pool.hpp"
#include "../memory_buffer.hpp"
#include "../surface.hpp"
#include "../drawing/painter.hpp"
#include "../drawing/image.hpp"

namespace Asgaard {
    namespace Widgets {
        class Cursor : public Surface {
        public:
            Cursor(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions, enum Theming::Theme::Elements cursorType)
                : Surface(id, screen, dimensions)
            {
                // create memory resources
                auto poolSize = (Dimensions().Width() * Dimensions().Height() * 4);
                m_memory = MemoryPool::Create(this, poolSize);
                
                m_buffer = MemoryBuffer::Create(this, m_memory, 0,
                    Dimensions().Width(), Dimensions().Height(),
                    PixelFormat::A8B8G8R8, MemoryBuffer::Flags::NONE);

                LoadCursor(cursorType);
            }
            
            Cursor(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions, const Drawing::Image& image)
                : Surface(id, screen, dimensions)
            {
                // create memory resources
                auto poolSize = (Dimensions().Width() * Dimensions().Height() * 4);
                m_memory = MemoryPool::Create(this, poolSize);
                
                m_buffer = MemoryBuffer::Create(this, m_memory, 0,
                    Dimensions().Width(), Dimensions().Height(),
                    PixelFormat::A8B8G8R8, MemoryBuffer::Flags::NONE);

                RenderImage(image);
            }

            void Show()
            {
                SetBuffer(m_buffer);
                ApplyChanges();
            }

            void Hide()
            {
                std::shared_ptr<MemoryBuffer> nullBuffer(nullptr);
                SetBuffer(nullBuffer);
                ApplyChanges();
            }

        private:
            void LoadCursor(enum Theming::Theme::Elements cursorType)
            {
                const auto theme = Theming::TM.GetTheme();
                auto cursor = theme->GetImage(cursorType);
                RenderImage(cursor);
            }

            void RenderImage(const Drawing::Image& image)
            {
                Drawing::Painter painter(m_buffer);
                painter.RenderImage(image);
            }   

        private:
            std::shared_ptr<MemoryPool>   m_memory;
            std::shared_ptr<MemoryBuffer> m_buffer;
        };
    }
}
