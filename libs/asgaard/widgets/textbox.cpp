/**
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
 
#include "../include/widgets/textbox.hpp"
#include "../include/drawing/painter.hpp"
#include "../include/drawing/font.hpp"
#include "../include/memory_pool.hpp"
#include "../include/memory_buffer.hpp"
#include "../include/rectangle.hpp"
#include "../include/events/key_event.hpp"
#include "../include/notifications/textchanged_notification.hpp"
#include <iostream>

using namespace Asgaard::Widgets;

Textbox::Textbox(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions)
    : SubSurface(id, screen, dimensions)
    , m_font(nullptr)
    , m_cursor(0)
    , m_text("")
    , m_placeholderText("")
    , m_textColor(0xFF, 0, 0, 0)
    , m_placeholderTextColor(0xFF, 0xBC, 0xBC, 0xBC)
    , m_fillColor(0xFF, 0xFF, 0xFF, 0xFF)
    , m_borderColor(0x0, 0xF0, 0xF0, 0xF0)
    , m_borderWidth(0)
    , m_imageAnchor(Anchor::LEFT)
    , m_image()
    , m_redraw(false)
    , m_redrawReady(false)
{
    auto poolSize = Dimensions().Width() * Dimensions().Height() * 4;
    m_memory = MemoryPool::Create(this, poolSize);

    m_buffer = MemoryBuffer::Create(this, m_memory, 0,
        Dimensions().Width(), Dimensions().Height(), 
        PixelFormat::A8B8G8R8, MemoryBuffer::Flags::NONE);

    MarkInputRegion(Dimensions());
    SetBuffer(m_buffer);
    RedrawReady();
}

Textbox::~Textbox()
{
    Destroy();
}

void Textbox::Destroy()
{
    if (m_memory) { m_memory->Unsubscribe(this); }
    if (m_buffer) { m_buffer->Unsubscribe(this); }

    // invoke base destroy
    SubSurface::Destroy();
}

void Textbox::SetBackgroundColor(const Drawing::Color& color)
{
    m_fillColor = color;
}

void Textbox::SetTextColor(const Drawing::Color& color)
{
    m_textColor = color;
}

void Textbox::SetFont(const std::shared_ptr<Drawing::Font>& font)
{
    m_font = font;
}

void Textbox::SetText(const std::string& text)
{
    m_text = text;
}

void Textbox::SetPlaceholderText(const std::string& text)
{
    m_placeholderText = text;
}

void Textbox::SetImage(const Drawing::Image& image)
{
    m_image = image;
}

void Textbox::SetImageAnchor(const Anchor anchor)
{
    m_imageAnchor = anchor;
}

void Textbox::SetBorder(const Drawing::Color& color)
{
    m_borderColor = color;
}

void Textbox::RequestRedraw()
{
    bool shouldRedraw = m_redrawReady.exchange(false);
    if (shouldRedraw) {
        Redraw();
    }
    else {
        m_redraw = true;
    }
}

void Textbox::RedrawReady()
{
    if (m_redraw) {
        Redraw();
        m_redraw = false;
    }
    else {
        m_redrawReady.store(true);
    }
}

void Textbox::Redraw()
{
    Drawing::Painter paint(m_buffer);

    const auto text = [&] {
        if (m_text.size()) {
            return m_text;
        }
        return m_placeholderText;
    }();

    const auto textColor = [&] {
        if (m_text.size()) {
            return m_textColor;
        }
        return m_placeholderTextColor;
    }();

    auto textDimensions = m_font->GetTextMetrics(text);
    
    constexpr auto PADDING = 4;
    const auto textX = [&] {
        if (m_image.Data() != nullptr) {
            return m_image.Width() + (2 * PADDING);
        }
        return PADDING;
    }();

    const auto textY = [&] {
        int centerDims = Dimensions().Height() >> 1;
        int centerText = textDimensions.Height() >> 1;
        return std::max(0, centerDims - centerText);
    }();

    auto renderImage = [&] {
        if (m_image.Data() != nullptr) {
            auto midY = (Dimensions().Height() >> 1) - (m_image.Height() >> 1); 
            paint.RenderImage(PADDING, midY, m_image);
        }
    };

    auto renderBorder = [&] {
        if (m_borderWidth) {
            paint.SetFillColor(m_borderColor);
            paint.RenderRectangle(Dimensions());
        }
    };

    paint.SetFillColor(m_fillColor);
    paint.RenderFill();
    
    renderImage();

    // render text
    paint.SetFont(m_font);
    paint.SetFillColor(m_fillColor);
    paint.SetOutlineColor(textColor);
    paint.RenderText(textX, textY, text);

    // render beam
    paint.SetFillColor(0xFF, 0, 0, 0);
    paint.RenderLine(textX + m_beamOffset, textY, textX + m_beamOffset, textY + textDimensions.Height());

    renderBorder();

    MarkDamaged(Dimensions());
    ApplyChanges();
}

void Textbox::CalculateBeamOffset()
{
    if (m_text.size() > 0) {
        auto substring = m_text.substr(0, m_cursor);
        auto textDimensions = m_font->GetTextMetrics(substring);
        m_beamOffset = textDimensions.Width();
    }
    else {
        m_beamOffset = 0;
    }
}

void Textbox::OnBackspace()
{
    if (m_cursor >= static_cast<int>(m_text.size())) {
        m_text.erase(m_cursor - 1, 1);
        m_cursor--;

        Notify(TextChangedNotification(Id(), m_text));
        CalculateBeamOffset();
        RequestRedraw();
    }
}

void Textbox::OnDelete()
{
    if (m_cursor >= 0 &&  m_cursor < static_cast<int>(m_text.size())) {
        m_text.erase(m_cursor, 1);

        Notify(TextChangedNotification(Id(), m_text));
        RequestRedraw();
    }
}

void Textbox::OnLeftArrow()
{
    if (m_cursor > 0) {
        m_cursor--;
        CalculateBeamOffset();
        RequestRedraw();
    }
}

void Textbox::OnRightArrow()
{
    if (m_cursor < static_cast<int>(m_text.size())) {
        m_cursor++;
        CalculateBeamOffset();
        RequestRedraw();
    }
}

void Textbox::AddInput(const KeyEvent& keyEvent)
{
    auto character = keyEvent.Key();
    if (!character) {
        return;
    }

    m_text.append(reinterpret_cast<const char*>(const_cast<const uint32_t*>(&character)));
    m_cursor++;

    Notify(TextChangedNotification(Id(), m_text));
    CalculateBeamOffset();
    RequestRedraw();
}

void Textbox::OnMouseEnter(const std::shared_ptr<Pointer>& pointer, int localX, int localY)
{
    pointer->SetDefaultSurface(Pointer::DefaultCursors::BEAM);
}

void Textbox::OnMouseLeave(const std::shared_ptr<Pointer>& pointer)
{
    pointer->SetDefaultSurface(Pointer::DefaultCursors::ARROW);
}

void Textbox::OnKeyEvent(const KeyEvent& keyEvent)
{
    std::cout << "Textbox::OnKeyEvent" << std::endl;

    switch (keyEvent.KeyCode()) {
        case VKC_BACK:
            OnBackspace();
            break;
        
        case VKC_DELETE:
            OnDelete();
            break;

        case VKC_LEFT:
            OnLeftArrow();
            break;
        
        case VKC_RIGHT:
            OnRightArrow();
            break;

        default:
            AddInput(keyEvent);
            break;
    }
}

void Textbox::OnFocus(bool focus)
{
    m_borderWidth = focus ? 1 : 0;
    RequestRedraw();
}

void Textbox::Notification(const Publisher* source, const Asgaard::Notification& notification)
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
