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
#include <drawing/font_manager.hpp>
#include <drawing/font.hpp>
#include <memory>
#include <string>
#include <ctime>

using namespace Asgaard;

class LauncherInfoSearch : public SubSurface {
public:
    LauncherInfoSearch(uint32_t id, const std::shared_ptr<Screen>& screen, const Surface* parent, const Rectangle& dimensions)
        : SubSurface(id, screen, parent, dimensions)
    {
        LoadResources();
        FinishSetup();
    }

    ~LauncherInfoSearch() {
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

        // load font
        m_font = Drawing::FM.CreateFont(DATA_DIRECTORY "/fonts/DejaVuSansMono.ttf", 14);
        m_smallFont = Drawing::FM.CreateFont(DATA_DIRECTORY "/fonts/DejaVuSansMono.ttf", 10);

        // create labels
        auto createLabel = [&] (int x, int y, int w, int h, Widgets::Label::Anchors anchors) {
            auto label = OM.CreateClientObject<Widgets::Label>(m_screen, this, Rectangle(x, y, w, h));
            label->SetFont(m_font);
            label->SetAnchors(anchors);
            label->SetBackgroundColor(theme->GetColor(Theming::Theme::Colors::DECORATION_FILL));
            label->SetTextColor(theme->GetColor(Theming::Theme::Colors::DECORATION_TEXT));
            return label;
        };

        // USERNAME LABEL starts __under__ the search widget
        // x => 15
        // y => widget + 50
        // w => width / 2
        auto midX = Dimensions().Width() >> 1;
        m_userLabel = createLabel(15, 50, midX - 15, 20, Widgets::Label::Anchors::LEFT);
        m_userLabel->SetText("Philip Meulengracht");

        m_pcNameLabel = createLabel(15, 70, midX - 15, 20, Widgets::Label::Anchors::LEFT);
        m_pcNameLabel->SetText("Vali-PC");

        m_timeLabel = createLabel(midX, 50, midX - 15, 20, Widgets::Label::Anchors::RIGHT);
        m_dateLabel = createLabel(midX, 70, midX - 15, 20, Widgets::Label::Anchors::RIGHT);

        m_sysinfoLabel = createLabel(midX, Dimensions().Height() - 25, midX - 15, 20, Widgets::Label::Anchors::RIGHT);
        m_sysinfoLabel->SetFont(m_smallFont);
        m_sysinfoLabel->SetText("Vali v0.7.0-dev");

        auto renderBackground = [&] {
            Drawing::Painter paint(m_buffer);
            
            // render background
            paint.SetFillColor(theme->GetColor(Theming::Theme::Colors::DECORATION_FILL));
            paint.RenderFill();

            // render input box
            paint.SetFillColor(Drawing::Color(0xFF, 0xFF, 0xFF, 0xFF));
            paint.RenderFill(Rectangle(15, 15, Dimensions().Width() - 30, 35));
        };
        renderBackground();
        UpdateTimeAndDate();
    }

    void FinishSetup()
    {
        SetBuffer(m_buffer);
        ApplyChanges();
    }

    void UpdateTimeAndDate()
    {
        std::time_t t   = std::time(0);   // get time now
        std::tm*    now = std::localtime(&t);
        char        buf[64] = { 0 };
        
        strftime(&buf[0], sizeof(buf), "%a, %H:%M:%S", now);
        m_timeLabel->SetText(std::string(&buf[0]));

        strftime(&buf[0], sizeof(buf), "%B %d, %Y", now);
        m_dateLabel->SetText(std::string(&buf[0]));
    }

private:
    std::shared_ptr<Asgaard::MemoryPool>   m_memory;
    std::shared_ptr<Asgaard::MemoryBuffer> m_buffer;
    std::shared_ptr<Drawing::Font>         m_font;
    std::shared_ptr<Drawing::Font>         m_smallFont;
    std::shared_ptr<Widgets::Label>        m_userLabel;
    std::shared_ptr<Widgets::Label>        m_pcNameLabel;
    std::shared_ptr<Widgets::Label>        m_timeLabel;
    std::shared_ptr<Widgets::Label>        m_dateLabel;
    std::shared_ptr<Widgets::Label>        m_sysinfoLabel;
};
