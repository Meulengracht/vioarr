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
#include <drawing/primitives/circle.hpp>
#include <widgets/label.hpp>
#include <widgets/icon.hpp>
#include <notifications/notification.hpp>
#include <memory>
#include <string>

#include "../utils/register.hpp"

using namespace Asgaard;

class ApplicationIcon : public SubSurface {
public:
    ApplicationIcon(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions, 
        const Heimdall::Register::Application& app) 
        : SubSurface(id, screen, dimensions)
        , m_name(app.GetName())
        , m_isShown(false)
        , m_isHovered(false)
        , m_redraw(false)
        , m_redrawReady(false)
    {
        LoadResources(app.GetIcon());
        MarkInputRegion(Dimensions());
        RedrawReady();
        Redraw();
        Show();
    }

    ~ApplicationIcon() {
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

        SetBuffer(std::shared_ptr<Asgaard::MemoryBuffer>(nullptr));
        ApplyChanges();

        // clear some values
        m_isShown = false;
        m_isHovered = false;
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

    bool IsShown() const { return m_isShown; }

private:
    void LoadResources(const Drawing::Image& image)
    {
        // this box is fixed, so we allocate just enough in this case
        auto screenSize = Dimensions().Width() * Dimensions().Height() * 4;
        m_memory = MemoryPool::Create(this, screenSize);

        // we create a transparent surface
        m_buffer = MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
            Dimensions().Height(), PixelFormat::A8B8G8R8, MemoryBuffer::Flags::NONE);

        // resize incoming icon
        m_icon = image.Resize(48, 48);
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
        paint.SetFillColor(0, 0, 0, 0);
        paint.RenderFill();

        // what we want is actually to draw the image here
        // and not keep it as a subsurface
        paint.SetRegion(Drawing::Primitives::CircleShape(32, 32, 24));
        paint.RenderImage(m_icon);

        // draw indicators
        //paint.RenderCircle()

        // if hovering, draw mirrors
    }

protected:
    void OnMouseEnter(const std::shared_ptr<Pointer>&, int localX, int localY) override
    {
        m_isHovered = true;
        //RequestRedraw();
    }

    void OnMouseLeave(const std::shared_ptr<Pointer>&) override
    {
        m_isHovered = false;
        //RequestRedraw();
    }

    void OnMouseClick(const std::shared_ptr<Pointer>&, enum Pointer::Buttons button, bool pressed) override
    {
        if (button == Pointer::Buttons::LEFT && !pressed) {
            // notify global surface
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
    Asgaard::Drawing::Image                m_icon;
    std::string                            m_name;
    bool                                   m_isShown;
    bool                                   m_isHovered;
    bool                                   m_redraw;
    std::atomic<bool>                      m_redrawReady;
};
