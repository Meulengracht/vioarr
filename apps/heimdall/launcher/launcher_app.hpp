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

#include <subsurface.hpp>
#include <memory_pool.hpp>
#include <memory_buffer.hpp>
#include <drawing/painter.hpp>
#include <widgets/label.hpp>
#include <widgets/icon.hpp>
#include <memory>
#include <string>

using namespace Asgaard;

class LauncherApplication : public SubSurface {
public:
    LauncherApplication(uint32_t id, const std::shared_ptr<Screen>& screen, const Surface* parent, const Rectangle& dimensions) 
        : SubSurface(id, screen, parent, dimensions)
    {
        LoadResources();
        FinishSetup();
    }

    ~LauncherApplication() {
        Destroy();
    }

    void Destroy() override
    {
        SubSurface::Destroy();
    }

private:
    void LoadResources()
    {
        // this box is fixed, so we allocate just enough in this case
        auto screenSize = Dimensions().Width() * Dimensions().Height() * 4;
        m_memory = MemoryPool::Create(this, screenSize);

        m_buffer = MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
            Dimensions().Height(), PixelFormat::X8B8G8R8, MemoryBuffer::Flags::NONE);

        const auto theme = Theming::TM.GetTheme();
        auto renderBackground = [&] {
            Drawing::Painter paint(m_buffer);
            paint.SetFillColor(theme->GetColor(Theming::Theme::Colors::DECORATION_FILL));
            paint.RenderFill();
        };

        // create labels

        // load font

        renderBackground();
    }

    void FinishSetup()
    {
        SetBuffer(m_buffer);
        ApplyChanges();
    }
    
private:
    std::shared_ptr<Asgaard::MemoryPool>   m_memory;
    std::shared_ptr<Asgaard::MemoryBuffer> m_buffer;
    std::shared_ptr<Drawing::Font>         m_font;
    std::shared_ptr<Widgets::Label>        m_label;
    std::shared_ptr<Widgets::Icon>         m_icon;
};
