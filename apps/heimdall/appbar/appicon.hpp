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

#include <gracht/server.h>
#include <asgaard/subsurface.hpp>
#include <asgaard/memory_pool.hpp>
#include <asgaard/memory_buffer.hpp>
#include <asgaard/drawing/painter.hpp>
#include <asgaard/drawing/primitives/rectangle.hpp>
#include <asgaard/drawing/primitives/circle.hpp>
#include <asgaard/widgets/label.hpp>
#include <asgaard/widgets/icon.hpp>
#include <asgaard/notifications/notification.hpp>
#include <asgaard/theming/theme_manager.hpp>
#include <asgaard/theming/theme.hpp>
#include <memory>
#include <string>
#include <vector>

using namespace Asgaard;

class ApplicationIcon : public SubSurface {
private:
    struct SourceObject {
        gracht_conn_t         source;
        std::vector<uint32_t> surfaces;
    };

public:
    ApplicationIcon(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions, 
        unsigned int applicationId, std::size_t memoryHandle, std::size_t size, int iconWidth, 
        int iconHeight, PixelFormat format)
        : SubSurface(id, screen, dimensions)
        , m_isShown(false)
        , m_isHovered(false)
        , m_redraw(false)
        , m_redrawReady(false)
    {
        LoadResources(memoryHandle, size, iconWidth, iconHeight, format);
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

    void AddSource(gracht_conn_t source)
    {
        auto sourceObject = new SourceObject();
        sourceObject->source = source;

        m_sources.push_back(sourceObject);
    }

    void RemoveSource(gracht_conn_t source)
    {
        auto entry = std::find_if(std::begin(m_sources), std::end(m_sources), 
            [source] (const SourceObject* i) { return i->source == source; });
        if (entry != std::end(m_sources)) {
            m_sources.erase(entry);
        }
    }

    int GetSourceCount()
    {
        return m_sources.size();
    }

    void AddSurface(gracht_conn_t source, uint32_t surfaceGlobalId)
    {
        auto sourceObject = GetSource(source);
        if (sourceObject) {
            sourceObject->surfaces.push_back(surfaceGlobalId);
        }
    }

    void RemoveSurface(gracht_conn_t source, uint32_t surfaceGlobalId)
    {
        auto sourceObject = GetSource(source);
        if (sourceObject) {
            auto entry = std::find(
                std::begin(sourceObject->surfaces),
                std::end(sourceObject->surfaces),
                surfaceGlobalId
            );
            if (entry != std::end(sourceObject->surfaces)) {
                sourceObject->surfaces.erase(entry);
            }
        }
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
    void LoadResources(std::size_t memoryHandle, std::size_t size, int iconWidth, 
        int iconHeight, PixelFormat format)
    {
        // this box is fixed, so we allocate just enough in this case
        auto screenSize = Dimensions().Width() * Dimensions().Height() * 4;
        m_memory = MemoryPool::Create(this, screenSize);

        // we create a transparent surface
        m_buffer = MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
            Dimensions().Height(), PixelFormat::A8B8G8R8, MemoryBuffer::Flags::NONE);

        // inherit the icon buffer
        if (memoryHandle != 0) {
            auto iconMemoryPool = MemoryPool::Inherit(this, memoryHandle, size);
            auto iconBuffer = MemoryBuffer::Create(this, iconMemoryPool, 0, iconWidth, iconHeight, format);
            Drawing::Image icon(iconBuffer->Buffer(), format, iconHeight, iconWidth, false);

            // resize incoming icon
            m_icon = icon.Resize(48, 48);

            // cleanup
            iconBuffer->Destroy();
            iconMemoryPool->Destroy();
        }
        else {
            // load default icon
            const auto theme = Theming::TM.GetTheme();
            auto icon = theme->GetImage(Theming::Theme::Elements::IMAGE_TERMINAL);

            // resize incoming icon
            m_icon = icon.Resize(48, 48);
        }
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
        Drawing::Primitives::RectangleShape iconShape(
            8, 4, // padding
            m_icon.Width(), m_icon.Height()
        );
        paint.SetRegion(&iconShape);
        paint.RenderImage(m_icon);

        // draw indicators
        paint.SetFillColor(0xFF, 0x6F, 0x6F, 0x6F);
        paint.RenderCircleFill(
            Dimensions().Width() >> 1,
            (m_icon.Height() + 4) + 6, 2
        );

        // if hovering, draw mirrors
    }

    SourceObject* GetSource(gracht_conn_t source)
    {
        auto entry = std::find_if(std::begin(m_sources), std::end(m_sources), 
            [source] (const SourceObject* i) { return i->source == source; });
        if (entry == std::end(m_sources)) {
            return nullptr;
        }
        return *entry;
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
            // transfer focus to first surface
            auto source = std::begin(m_sources);
            while (source != std::end(m_sources)) {
                auto surface = std::begin((*source)->surfaces);
                if (surface != std::end((*source)->surfaces)) {
                    TransferFocus(*surface);
                    break;
                }
                source = std::next(source);
            }
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
    std::vector<SourceObject*>             m_sources;
    bool                                   m_isShown;
    bool                                   m_isHovered;
    bool                                   m_redraw;
    std::atomic<bool>                      m_redrawReady;
};
