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

#include <application.hpp>
#include <window_base.hpp>
#include <memory_pool.hpp>
#include <memory_buffer.hpp>
#include <object_manager.hpp>
#include <events/key_event.hpp>
#include <drawing/painter.hpp>
#include <theming/theme_manager.hpp>
#include <theming/theme.hpp>
#include <keycodes.h>

#include "effects/guassian_blur.hpp"
#include "widgets/cursor.hpp"

#ifdef MOLLENOS
#include <ddk/utils.h>
#include <os/process.h>
#else
#include <unistd.h>
#endif

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
        Drawing::Image background(DATA_DIRECTORY "/themes/backgrounds/bg.png");
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
#ifdef MOLLENOS
        ERROR("Heimdall::OnKeyEvent(keycode=%u)", keyEvent.KeyCode());
#endif

        if (keyEvent.KeyCode() == VKC_F1 && !keyEvent.Pressed()) {
#ifdef MOLLENOS
            UUId_t pid;
            ProcessSpawn("$bin/alumni.app", NULL, &pid);
#else
            auto childPid = fork();
            if (!childPid) {
                char* argv[] = { "alumni", NULL };
                auto  resultCode = execv("alumni", argv);
                exit(resultCode);
            }
#endif
        }
        else if (keyEvent.KeyCode() == VKC_LWIN && !keyEvent.Pressed()) {
            // Show 
        }
    }
    
private:
    std::shared_ptr<MemoryPool>     m_memory;
    std::shared_ptr<MemoryBuffer>   m_buffer;
    std::shared_ptr<HeimdallCursor> m_cursor;
};
