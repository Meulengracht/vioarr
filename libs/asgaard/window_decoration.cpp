/* ValiOS
 *
 * Copyright 2020, Philip Meulengracht
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

#include "include/object_manager.hpp"
#include "include/memory_pool.hpp"
#include "include/memory_buffer.hpp"
#include "include/rectangle.hpp"
#include "include/pointer.hpp"
#include "include/window_decoration.hpp"
#include "include/window_title.hpp"
#include "include/theming/theme_manager.hpp"
#include "include/theming/theme.hpp"
#include "include/drawing/painter.hpp"
#include "include/drawing/font_manager.hpp"
#include "include/drawing/font.hpp"
#include "include/drawing/image.hpp"

#include "include/widgets/icon.hpp"
#include "include/widgets/label.hpp"

#define ICON_SIZE 16

namespace Asgaard {
    WindowDecoration::WindowDecoration(uint32_t id, const std::shared_ptr<Screen>& screen, const Surface* parent, const Rectangle& dimensions)
        : WindowDecoration(id, screen, parent, dimensions, Drawing::FM.CreateFont("$sys/fonts/DejaVuSansMono.ttf", DECORATION_TEXT_SIZE)) { }

    WindowDecoration::WindowDecoration(uint32_t id, 
        const std::shared_ptr<Screen>& screen,
        const Surface* parent,
        const Rectangle& dimensions,
        const std::shared_ptr<Drawing::Font>& font) 
        : SubSurface(id, screen, parent, dimensions)
        , m_appFont(font)
        , m_redraw(false)
        , m_redrawReady(false)
    {
        // create memory resources
        auto poolSize = (Dimensions().Width() * Dimensions().Height() * 4);
        m_memory = MemoryPool::Create(this, poolSize);
        
        m_buffer = MemoryBuffer::Create(this, m_memory, 0,
            Dimensions().Width(), Dimensions().Height(),
            PixelFormat::A8B8G8R8, MemoryBuffer::Flags::NONE);
        
        SetTransparency(true);
        SetBuffer(m_buffer);
        RedrawReady();

        Initialize();
    }
    
    WindowDecoration::~WindowDecoration()
    {
        Destroy();
    }

    void WindowDecoration::Destroy()
    {
        if (m_memory)    { m_memory->Unsubscribe(this); }
        if (m_buffer)    { m_buffer->Unsubscribe(this); }
        if (m_appTitle)  { m_appTitle->Unsubscribe(this); }
        if (m_appIcon)   { m_appIcon->Unsubscribe(this); }
        if (m_minIcon)   { m_minIcon->Unsubscribe(this); }
        if (m_maxIcon)   { m_maxIcon->Unsubscribe(this); }
        if (m_closeIcon) { m_closeIcon->Unsubscribe(this); }

        // invoke base destroy
        SubSurface::Destroy();
    }

    void WindowDecoration::Initialize()
    {
        float halfHeight = (float)Dimensions().Height() / 2.0f;
        int        iconY = (int)(halfHeight - (ICON_SIZE / 2.0f));
        const auto theme = Theming::TM.GetTheme();

        // load icons
        Drawing::Image appImage   = theme->GetImage(Theming::Theme::Elements::IMAGE_APP_DEFAULT);
        Drawing::Image minImage   = theme->GetImage(Theming::Theme::Elements::IMAGE_MINIMIZE);
        Drawing::Image maxImage   = theme->GetImage(Theming::Theme::Elements::IMAGE_MAXIMIZE);
        Drawing::Image closeImage = theme->GetImage(Theming::Theme::Elements::IMAGE_CLOSE);

        // left corner
        m_appIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_screen, this,
            Rectangle(8, (int)(halfHeight - (ICON_SIZE / 2.0f)), ICON_SIZE, ICON_SIZE));
        m_appIcon->Subscribe(this);
        m_appIcon->SetImage(appImage);

        // middle
        m_appTitle = OM.CreateClientObject<Asgaard::WindowTitle>(m_screen, this,
            Rectangle(
                8 + 8 + ICON_SIZE, // start text next to app icon
                0, 
                Dimensions().Width() - ((3 * (8 + ICON_SIZE)) + 8 + 8 + 8 + ICON_SIZE),
                Dimensions().Height()));
        m_appTitle->SetFont(m_appFont);
        m_appTitle->SetAnchors(Widgets::Label::Anchors::CENTER);
        m_appTitle->SetBackgroundColor(DECORATION_FILL_COLOR);
        m_appTitle->SetTextColor(DECORATION_TEXT_COLOR);
        m_appTitle->Subscribe(this);
        
        // right corner
        m_minIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_screen, this,
            Rectangle(Dimensions().Width() - (3 * (8 + ICON_SIZE)), 8.0f, ICON_SIZE, ICON_SIZE));
        m_minIcon->Subscribe(this);
        m_minIcon->SetImage(minImage);

        // right corner
        m_maxIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_screen, this,
            Rectangle(Dimensions().Width() - (2 * (8 + ICON_SIZE)), 8.0f, ICON_SIZE, ICON_SIZE));
        m_maxIcon->Subscribe(this);
        m_maxIcon->SetImage(maxImage);

        // right corner
        m_closeIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_screen, this,
            Rectangle(Dimensions().Width() - (8 + ICON_SIZE), 8.0f, ICON_SIZE, ICON_SIZE));
        m_closeIcon->Subscribe(this);
        m_closeIcon->SetImage(closeImage);
    }
    
    void WindowDecoration::SetTitle(const std::string& title)
    {
        m_appTitle->SetText(title);
        m_appTitle->RequestRedraw();
    }
    
    void WindowDecoration::SetImage(const std::shared_ptr<Drawing::Image>& image)
    {
        m_appIcon->SetImage(*image.get());
    }

    void WindowDecoration::SetVisible(bool visible)
    {
        if (visible) {
            SetBuffer(m_buffer);
            RequestRedraw();
        }
        else {
            auto nullp = std::shared_ptr<MemoryBuffer>(nullptr);
            SetBuffer(nullp);
            RequestRedraw();
        }
    }
    
    void WindowDecoration::Redraw()
    {
        Drawing::Painter paint(m_buffer);
        paint.SetFillColor(DECORATION_FILL_COLOR);
        paint.RenderFill();
        
        MarkDamaged(Dimensions());
        ApplyChanges();
    }

    void WindowDecoration::RequestRedraw()
    {
        bool shouldRedraw = m_redrawReady.exchange(false);
        if (shouldRedraw) {
            Redraw();
        }
        else {
            m_redraw = true;
        }
    }

    void WindowDecoration::RedrawReady()
    {
        // Request redraw
        if (m_redraw) {
            Redraw();
            m_redraw = false;
        }
        else {
            m_redrawReady.store(true);
        }
    }
    
    void WindowDecoration::Notification(Publisher* source, int event, void* data)
    {
        auto object = dynamic_cast<Object*>(source);
        if (object == nullptr) {
            return;
        }

        if (event == static_cast<int>(Object::Notification::ERROR)) {
            Notify(static_cast<int>(Object::Notification::ERROR), data);
        }
        else if (object->Id() == m_appTitle->Id()) {
            if (event == static_cast<int>(WindowTitle::Notification::INITIATE_DRAG)) {
                Notify(static_cast<int>(Notification::INITIATE_DRAG), data);
            }
        }
        else if (event == static_cast<int>(Widgets::Icon::Notification::CLICKED)) {
            if (object->Id() == m_minIcon->Id()) {
                Notify(static_cast<int>(Notification::MINIMIZE));
            }
            else if (object->Id() == m_maxIcon->Id()) {
                Notify(static_cast<int>(Notification::MAXIMIZE));
            }
            else if (object->Id() == m_closeIcon->Id()) {
                exit(EXIT_SUCCESS);
            }
        }
    }
}
