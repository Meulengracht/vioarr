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
 * ValiOS - Application Environment (Launcher)
 *  - Contains the implementation of the application environment to support
 *    graphical user interactions.
 */
#pragma once

#include <asgaard/memory_pool.hpp>
#include <asgaard/memory_buffer.hpp>
#include <asgaard/surface.hpp>

using namespace Asgaard;

class HeimdallCursor : public Surface {
public:
    HeimdallCursor(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions)
        : Surface(id, screen, dimensions)
    {
        // create memory resources
        auto poolSize = (Dimensions().Width() * Dimensions().Height() * 4);
        m_memory = MemoryPool::Create(this, poolSize);
        
        m_buffer = MemoryBuffer::Create(this, m_memory, 0,
            Dimensions().Width(), Dimensions().Height(),
            PixelFormat::A8B8G8R8, MemoryBuffer::Flags::NONE);

        SetTransparency(true);
        LoadCursor();
    }

    void LoadCursor()
    {
        const auto theme = Theming::TM.GetTheme();
        auto cursor = theme->GetImage(Theming::Theme::Elements::IMAGE_CURSOR);

        auto renderImage = [&](const auto& buffer, const auto& image) {
            Asgaard::Drawing::Painter painter(buffer);
            painter.RenderImage(image);
        };
        renderImage(m_buffer, cursor);
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
    std::shared_ptr<MemoryPool>   m_memory;
    std::shared_ptr<MemoryBuffer> m_buffer;
};
