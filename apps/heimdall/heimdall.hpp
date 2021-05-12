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

#include <asgaard/application.hpp>
#include <asgaard/window_base.hpp>
#include <asgaard/memory_pool.hpp>
#include <asgaard/memory_buffer.hpp>
#include <asgaard/object_manager.hpp>
#include <asgaard/key_event.hpp>
#include <asgaard/drawing/painter.hpp>
#include <asgaard/theming/theme_manager.hpp>
#include <asgaard/theming/theme.hpp>
#include <os/keycodes.h>
#include <os/process.h>

#include "effects/guassian_blur.hpp"
#include "widgets/cursor.hpp"

#include <ddk/utils.h>

#define CURSOR_SIZE 16

using namespace Asgaard;

class Heimdall final : public WindowBase {
public:
    Heimdall(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions)
        : WindowBase(id, screen, dimensions) { }
    
    ~Heimdall()
    {
        // Override destroy
    }
    
private:
    void OnCreated() override
    {
        auto screenSize = m_screen->GetCurrentWidth() * m_screen->GetCurrentHeight() * 4;
        m_memory = MemoryPool::Create(this, screenSize);

        m_buffer = MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
            Dimensions().Height(), PixelFormat::X8B8G8R8, MemoryBuffer::Flags::NONE);
        
        m_cursor = OM.CreateClientObject<HeimdallCursor>(m_screen, Rectangle(0, 0, CURSOR_SIZE, CURSOR_SIZE));

        // Now all objects are created, load and prepare resources
        LoadResources();
        FinishSetup();
    }

    void LoadResources()
    {
        auto renderImage = [](const auto& buffer, const auto& image) {
            Drawing::Painter painter(buffer);
            painter.RenderImage(image);
        };

        auto blurImage = [&](const auto& buffer, const auto& image) {
            GuassianBlurEffect blurEffect;
            auto blurredBackground = blurEffect.Apply(image, 3.0);
            renderImage(buffer, blurredBackground);
        };

        // Render the normal state
        Drawing::Image background("$themes/backgrounds/bg.png");
        renderImage(m_buffer, background);
        //blurImage(m_bufferBlurred, background);
    }
    
    void FinishSetup()
    {
        RequestPriorityLevel(PriorityLevel::BOTTOM);
        RequestFullscreenMode(FullscreenMode::NORMAL);
        MarkInputRegion(Dimensions());
        SetBuffer(m_buffer);
        MarkDamaged(Dimensions());
        ApplyChanges();
    }
    
    void OnRefreshed(MemoryBuffer* buffer) override
    {
        // nothing to do
    }

    void OnMouseEnter(const std::shared_ptr<Pointer>& pointer, int localX, int localY) override
    {
        // When a new cursor image is set the old surface is automatically cleared of its buffer
        m_cursor->Show();
        pointer->SetSurface(m_cursor);
    }

    void OnMouseLeave(const std::shared_ptr<Pointer>&) override
    {

    }

    void OnMouseMove(const std::shared_ptr<Pointer>& pointer, int localX, int localY) override
    {

    }

    void OnMouseClick(const std::shared_ptr<Pointer>&, enum Pointer::Buttons button, bool pressed) override
    {

    }
    
    void OnKeyEvent(const KeyEvent& keyEvent) override
    {
        ERROR("Heimdall::OnKeyEvent(keycode=%u", keyEvent.KeyCode());
        if (keyEvent.KeyCode() == VK_F1 && !keyEvent.Pressed()) {
            UUId_t pid;
            ProcessSpawn("$bin/alumni.app", NULL, &pid);
        }
        else if (keyEvent.KeyCode() == VK_LWIN && !keyEvent.Pressed()) {
            // Show 
        }
    }
    
private:
    std::shared_ptr<MemoryPool>     m_memory;
    std::shared_ptr<MemoryBuffer>   m_buffer;
    std::shared_ptr<HeimdallCursor> m_cursor;
};
