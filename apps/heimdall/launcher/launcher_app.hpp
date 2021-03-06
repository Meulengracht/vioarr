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

#include <asgaard/subsurface.hpp>
#include <asgaard/memory_pool.hpp>
#include <asgaard/memory_buffer.hpp>
#include <asgaard/drawing/painter.hpp>
#include <asgaard/drawing/primitives/rectangle.hpp>
#include <asgaard/widgets/label.hpp>
#include <asgaard/widgets/icon.hpp>
#include <asgaard/theming/theme_manager.hpp>
#include <asgaard/theming/theme.hpp>
#include <memory>
#include <string>

#include <asgaard/notifications/notification.hpp>

using namespace Asgaard;

class LauncherApplication : public SubSurface {
public:
    LauncherApplication(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions, 
        const std::string& name, const Drawing::Image& image) 
        : SubSurface(id, screen, dimensions)
        , m_name(name)
        , m_isShown(false)
        , m_isHovered(false)
        , m_redraw(false)
        , m_redrawReady(false)
    {
        LoadResources(image);
        MarkInputRegion(Dimensions());
        SetDropShadow(Asgaard::Rectangle(-10, -10, 20, 30));
        RedrawReady();
        Redraw();
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

        // clear some values
        m_isShown = false;
        m_isHovered = false;
    }

    void SetHighlight(bool highlight)
    {
        m_isHovered = highlight;
    }

    void RequestRedraw()
    {
        bool shouldRedraw = m_redrawReady.exchange(false);
        if (shouldRedraw) {
            Redraw();
            MarkDamaged(Dimensions());
            ApplyChanges();
        }
        else {
            m_redraw = true;
        }
    }

    std::string const& GetName() const { return m_name; }
    bool               IsShown() const { return m_isShown; }

private:
    void LoadResources(const Drawing::Image& image)
    {
        // this box is fixed, so we allocate just enough in this case
        auto screenSize = Dimensions().Width() * Dimensions().Height() * 4;
        m_memory = MemoryPool::Create(this, screenSize);

        m_buffer = MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
            Dimensions().Height(), PixelFormat::X8B8G8R8, MemoryBuffer::Flags::NONE);

        // create resources
        m_font = Drawing::FM.CreateFont(DATA_DIRECTORY "/fonts/DejaVuSansMono.ttf", 12);
        m_image = image.Resize(48, 48);

        auto textDimensions = m_font->GetTextMetrics(m_name);

        int centerDims = Dimensions().Width() >> 1;
        int centerText = textDimensions.Width() >> 1;
        m_nameX = std::max(0, centerDims - centerText);

        centerDims = Dimensions().Height() - (20 / 2);
        centerText = textDimensions.Height() >> 1;
        m_nameY = std::max(0, centerDims - centerText);
    }

    void RedrawReady()
    {
        if (m_redraw) {
            Redraw();
            MarkDamaged(Dimensions());
            ApplyChanges();
            m_redraw = false;
        }
        else {
            m_redrawReady.store(true);
        }
    }

    void Redraw()
    {
        Drawing::Painter paint(m_buffer);
        const auto theme = Theming::TM.GetTheme();
        
        auto renderFill = [&] {
            if (m_isHovered) {
                paint.SetFillColor(theme->GetColor(Theming::Theme::Colors::DEFAULT_HIGHLIGHT));
            }
            else {
                paint.SetFillColor(theme->GetColor(Theming::Theme::Colors::DEFAULT_FILL));
            }
            paint.RenderFill();
        };

        auto renderImage = [&] {
            Drawing::Primitives::RectangleShape imageShape(8, 8, m_image.Width(), m_image.Height());
            paint.SetRegion(&imageShape);
            paint.RenderImage(m_image);
            paint.SetRegion(nullptr);
        };

        auto renderText = [&] {
            paint.SetFont(m_font);
            paint.SetOutlineColor(theme->GetColor(Theming::Theme::Colors::DECORATION_TEXT));
            paint.RenderText(m_nameX, m_nameY, m_name);
        };

        renderFill();
        renderImage();
        renderText();
    }

protected:
    void OnMouseEnter(const std::shared_ptr<Pointer>&, int localX, int localY) override
    {
        m_isHovered = true;
        RequestRedraw();
    }

    void OnMouseLeave(const std::shared_ptr<Pointer>&) override
    {
        m_isHovered = false;
        RequestRedraw();
    }

    void OnMouseClick(const std::shared_ptr<Pointer>&, enum Pointer::Buttons button, bool pressed) override
    {
        if (button == Pointer::Buttons::LEFT && !pressed) {
            Notify(ClickedNotification(Id()));
        }
    }

    void Notification(const Publisher* source, const Asgaard::Notification& notification) override
    {
        switch (notification.GetType())
        {
            case NotificationType::ERROR: {
                // log error
            } break;

            case NotificationType::REFRESHED: {
                RedrawReady();
            } break;

            default: break;
        }

        SubSurface::Notification(source, notification);
    }


private:
    std::shared_ptr<Asgaard::MemoryPool>   m_memory;
    std::shared_ptr<Asgaard::MemoryBuffer> m_buffer;
    std::shared_ptr<Drawing::Font>         m_font;
    Drawing::Image                         m_image;
    std::string                            m_name;
    int                                    m_nameX;
    int                                    m_nameY;
    bool                                   m_isShown;
    bool                                   m_isHovered;
    bool                                   m_redraw;
    std::atomic<bool>                      m_redrawReady;
};
