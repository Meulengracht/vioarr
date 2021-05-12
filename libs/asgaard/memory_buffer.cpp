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

#include "include/application.hpp"
#include "include/exceptions/application_exception.h"
#include "include/exceptions/invalid_argument_exception.h"
#include "include/exceptions/out_of_range_exception.h"
#include "include/memory_pool.hpp"
#include "include/memory_buffer.hpp"

#include "wm_core_protocol_client.h"
#include "wm_memory_pool_protocol_client.h"
#include "wm_buffer_protocol_client.h"


static int CalculateStride(int width, enum Asgaard::PixelFormat format)
{
    return width * GetBytesPerPixel(format);
}

static enum wm_pixel_format GetWmPixelFormat(enum Asgaard::PixelFormat format)
{
    switch (format) {
        case Asgaard::PixelFormat::A8R8G8B8: return format_a8r8g8b8;
        case Asgaard::PixelFormat::A8B8G8R8: return format_a8b8g8r8;
        case Asgaard::PixelFormat::X8R8G8B8: return format_x8r8g8b8;
        case Asgaard::PixelFormat::X8B8G8R8: return format_x8b8g8r8;
        case Asgaard::PixelFormat::R8G8B8A8: return format_r8g8b8a8;
        case Asgaard::PixelFormat::B8G8R8A8: return format_b8g8r8a8;
    }
}

Asgaard::MemoryBuffer::MemoryBuffer(uint32_t id, const std::shared_ptr<MemoryPool>& memory, 
                            int memoryOffset, int width, int height, 
                            enum PixelFormat format, enum Flags flags)
    : Object(id)
    , m_memory(memory)
    , m_width(width)
    , m_height(height)
    , m_format(format)
    , m_flags(flags)
    , m_buffer(memory->CreateBufferPointer(memoryOffset))
{
    enum wm_pixel_format wmFormat = GetWmPixelFormat(format);
    int                  stride   = CalculateStride(width, format);
    if (stride <= 0) {
        throw InvalidArgumentException("MemoryBuffer::MemoryBuffer() invalid width provided");
    }
    if (height <= 0) {
        throw InvalidArgumentException("MemoryBuffer::MemoryBuffer() invalid height provided");
    }
    if (memoryOffset < 0) {
        throw InvalidArgumentException("MemoryBuffer::MemoryBuffer() invalid memoryOffset provided");
    }
    if (m_buffer == nullptr) {
        throw ApplicationException("MemoryBuffer::MemoryBuffer() buffer pointer was null", EINVAL);
    }

    if ((memoryOffset + (stride * height)) > memory->Size()) {
        throw ApplicationException("MemoryBuffer::MemoryBuffer() size too large would overrun the memory in the pool", EINVAL);
    }
    
    wm_memory_pool_create_buffer(APP.GrachtClient(), nullptr, memory->Id(),
        id, memoryOffset, width, height, stride, wmFormat, 
        static_cast<unsigned int>(flags));
}

Asgaard::MemoryBuffer::~MemoryBuffer()
{
    wm_buffer_destroy(APP.GrachtClient(), nullptr, Id());
}

void* Asgaard::MemoryBuffer::Buffer(int x, int y) {
    uint8_t* pointer       = static_cast<uint8_t*>(m_buffer);
    int      bytesPerPixel = GetBytesPerPixel(m_format);
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        throw OutOfRangeException("MemoryBuffer::Buffer(x, y) arguments were out of range");
    }
    
    pointer += ((y * (m_width * bytesPerPixel)) + (x * bytesPerPixel));
    return pointer;
}

void Asgaard::MemoryBuffer::ExternalEvent(enum ObjectEvent event, void* data)
{
    if (event == ObjectEvent::BUFFER_RELEASE) {
        Notify(static_cast<int>(Notification::REFRESHED));
        return;
    }

    Object::ExternalEvent(event, data);
}
