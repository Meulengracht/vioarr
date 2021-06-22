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
#include <notifications/textchanged_notification.hpp>
#include <drawing/painter.hpp>
#include <widgets/label.hpp>
#include <theming/theme_manager.hpp>
#include <theming/theme.hpp>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <map>

using namespace Asgaard;

class Notification : public Surface {
public:
    StatusBar(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions)
        : Surface(id, screen, dimensions)
    {
        LoadResources();
        Configure();
    }

    ~StatusBar() {
        Destroy();
    }

    void Destroy() override
    {
        Surface::Destroy();
    }

public:
    void LoadResources()
    {
        auto statusBarSize = Dimensions().Width() * Dimensions().Height() * 4;
        m_memory = MemoryPool::Create(this, statusBarSize);

        m_buffer = MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
            Dimensions().Height(), PixelFormat::X8B8G8R8, MemoryBuffer::Flags::NONE);

        // create font
        m_font = Drawing::FM.CreateFont(DATA_DIRECTORY "/fonts/DejaVuSansMono.ttf", 12);

        // create labels
        constexpr auto LABEL_WIDTH = 200.0f;
        auto midX = (Dimensions().Width() / 2.0f) - (LABEL_WIDTH / 2.0f);
        m_dateAndTime = SubSurface::Create<Widgets::Label>(
            this, 
            Rectangle(
                static_cast<int>(midX), 0, 
                static_cast<int>(LABEL_WIDTH), Dimensions().Height()
            )
        );
        m_dateAndTime->SetFont(m_font);
        m_dateAndTime->SetText("Jun 17, 12:00");
        m_dateAndTime->SetAnchors(Widgets::Label::Anchors::CENTER);
        m_dateAndTime->RequestRedraw();

        auto render = [&] {
            Drawing::Painter painter(m_buffer);

            const auto theme = Theming::TM.GetTheme();
            painter.SetFillColor(theme->GetColor(Theming::Theme::Colors::DEFAULT_FILL));
            painter.RenderFill();
        };
        render();
    }

    void Configure()
    {
        RequestPriorityLevel(PriorityLevel::TOP);
        MarkInputRegion(Dimensions());
        SetBuffer(m_buffer);
        ApplyChanges();
    }

private:
    std::shared_ptr<Asgaard::MemoryPool>   m_memory;
    std::shared_ptr<Asgaard::MemoryBuffer> m_buffer;
    std::shared_ptr<Drawing::Font>         m_font;
    std::shared_ptr<Widgets::Label>        m_dateAndTime;
};
