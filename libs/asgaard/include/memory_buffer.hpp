/* ValiOS
 *
 * Copyright 2018, Philip Meulengracht
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
#pragma once

#include "config.hpp"
#include "pixel_format.hpp"
#include "object_manager.hpp"
#include "object.hpp"
#include "utils/bitset_enum.hpp"

namespace Asgaard {
    class MemoryPool;
    
    class MemoryBuffer final : public Object {
    public:
        enum class Notification : int {
            REFRESHED = static_cast<int>(Object::Notification::CUSTOM_START),
        };
        
        enum class Flags : int {
            NONE = 0,
            FLIP_Y
        };

    public:
        ASGAARD_API MemoryBuffer(uint32_t id, const std::shared_ptr<MemoryPool>& memory, int memoryOffset,
            int width, int height, enum PixelFormat format, enum Flags flags);
        ASGAARD_API ~MemoryBuffer();
        
        void*             Buffer() const { return m_buffer; }
        ASGAARD_API void* Buffer(int x, int y);
        
        int         Width()  const { return m_width; }
        int         Height() const { return m_height; }
        int         Stride() const { return m_width * GetBytesPerPixel(m_format); }
        PixelFormat Format() const { return m_format; }
        
    public:
        static std::shared_ptr<MemoryBuffer> Create(Object* owner, const std::shared_ptr<MemoryPool>& memory,
            int memoryOffset, int width, int height, enum PixelFormat format, enum Flags flags)
        {
            auto buffer = OM.CreateClientObject<
                MemoryBuffer, const std::shared_ptr<MemoryPool>&, int, int, int, enum PixelFormat>(
                    memory, memoryOffset, width, height, format, flags);
            buffer->Subscribe(owner);
            return buffer;
        }
    
    public:
        void ExternalEvent(enum ObjectEvent event, void* data = 0) final;
        
    private:
        std::shared_ptr<MemoryPool> m_memory;
        int                         m_width;
        int                         m_height;
        enum PixelFormat            m_format;
        enum Flags                  m_flags;
        void*                       m_buffer;
    };
}

// Enable bitset operations for the anchors enum
template<>
struct enable_bitmask_operators<Asgaard::MemoryBuffer::Flags> {
    static const bool enable = true;
};
