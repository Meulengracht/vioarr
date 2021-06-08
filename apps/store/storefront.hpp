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
#include <widgets/cursor.hpp>
#include <keycodes.h>

#ifdef MOLLENOS
#include <ddk/utils.h>
#include <os/process.h>
#else
#include <unistd.h>
#endif

#define CURSOR_SIZE 16

using namespace Asgaard;

class Storefront final : public WindowBase {
public:
    Storefront(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions)
        : WindowBase(id, screen, dimensions) { }
    
    ~Storefront()
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
        
        // Now all objects are created, load and prepare resources
        LoadResources();
        FinishSetup();
    }

    void LoadResources()
    {
        
    }
    
    void FinishSetup()
    {
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
        
    }

    void OnKeyEvent(const KeyEvent& keyEvent) override
    {
        
    }
    
private:
    std::shared_ptr<MemoryPool>      m_memory;
    std::shared_ptr<MemoryBuffer>    m_buffer;
};
