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
#include <theming/theme_manager.hpp>
#include <theming/theme.hpp>
#include <memory>
#include <string>

using namespace Asgaard;

class LauncherApplication : public SubSurface {
public:
    LauncherApplication(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions, 
        const std::string& name, const Drawing::Image& image) 
        : SubSurface(id, screen, dimensions)
        , m_name(name)
        , m_isShown(false)
    {
        LoadResources(image);
        MarkInputRegion(Dimensions());
        Show();
    }

    ~LauncherApplication() {
        Destroy();
    }

    void Destroy() override
    {
        SubSurface::Destroy();
    }

    void Show()
    {
        if (m_isShown) {
            return;
        }

        SetBuffer(m_buffer);
        ApplyChanges();
        m_isShown = true;
    }

    void Hide()
    {
        if (!m_isShown) {
            return;
        }

        std::shared_ptr<Asgaard::MemoryBuffer> empty(nullptr);
        SetBuffer(empty);
        ApplyChanges();
        m_isShown = false;
    }

    std::string const& GetName() const { return m_name; }

private:
    void LoadResources(const Drawing::Image& image)
    {
        // this box is fixed, so we allocate just enough in this case
        auto screenSize = Dimensions().Width() * Dimensions().Height() * 4;
        m_memory = MemoryPool::Create(this, screenSize);

        m_buffer = MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
            Dimensions().Height(), PixelFormat::X8B8G8R8, MemoryBuffer::Flags::NONE);

        const auto theme = Theming::TM.GetTheme();
        auto renderBackground = [&] {
            Drawing::Painter paint(m_buffer);
            paint.SetFillColor(theme->GetColor(Theming::Theme::Colors::DEFAULT_FILL));
            paint.RenderFill();
        };

        // create font
        m_font = Drawing::FM.CreateFont(DATA_DIRECTORY "/fonts/DejaVuSansMono.ttf", 14);

        // create labels
        m_label = SubSurface::Create<Widgets::Label>(
            this, 
            Rectangle(0, Dimensions().Height() - 15, Dimensions().Width(), 15)
        );
        m_label->SetAnchors(Widgets::Label::Anchors::CENTER);
        m_label->SetText(m_name);
        m_label->SetFont(m_font);
        m_label->SetBackgroundColor(theme->GetColor(Theming::Theme::Colors::DEFAULT_FILL));
        m_label->SetTextColor(theme->GetColor(Theming::Theme::Colors::DECORATION_TEXT));

        // create image
        m_icon = SubSurface::Create<Widgets::Icon>(
            this,
            Rectangle(8, 8, 48, 48)
        );
        m_icon->SetImage(image);

        renderBackground();
    }

    protected:
        void OnMouseEnter(const std::shared_ptr<Pointer>&, int localX, int localY) override
        {
            // we must draw borderS!
        }

        void OnMouseLeave(const std::shared_ptr<Pointer>&) override
        {
            // we must not draw borderS!
        }

        void OnMouseClick(const std::shared_ptr<Pointer>&, enum Pointer::Buttons button, bool pressed) override
        {
            // cleaR!

            // send event
        }

private:
    std::shared_ptr<Asgaard::MemoryPool>   m_memory;
    std::shared_ptr<Asgaard::MemoryBuffer> m_buffer;
    std::shared_ptr<Drawing::Font>         m_font;
    std::shared_ptr<Widgets::Label>        m_label;
    std::shared_ptr<Widgets::Icon>         m_icon;
    std::string                            m_name;
    bool                                   m_isShown;
};
