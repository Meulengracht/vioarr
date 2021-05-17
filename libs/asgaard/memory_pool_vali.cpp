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
#include "include/memory_pool.hpp"

#include "wm_core_protocol_client.h"
#include "wm_memory_pool_protocol_client.h"
#include "wm_memory_protocol_client.h"

#include <os/mollenos.h>

namespace Asgaard {
    MemoryPool::MemoryPool(uint32_t id, std::size_t size)
        : Object(id)
        , m_size(size)
    {
        if (size == 0) {
            throw InvalidArgumentException("MemoryPool::MemoryPool 0-size provided");
        }

        // create the dma buffer that should be shared with the WM
        struct dma_buffer_info info;
        info.name = "asgaard_pool";
        info.capacity = size;
        info.length = size;
        info.flags = DMA_CLEAN;

        auto status = dma_create(&info, &m_attachment);
        if (status != OsSuccess) {
            throw ApplicationException("MemoryPool::MemoryPool() failed to create a new memory pool", OsStatusToErrno(status));
        }

        wm_memory_create_pool(APP.GrachtClient(), nullptr, id, m_attachment.handle, size);
    }
    
    MemoryPool::~MemoryPool()
    {
        if (m_attachment.buffer) {
            dma_attachment_unmap(&m_attachment);
            dma_detach(&m_attachment);
        }
        wm_memory_pool_destroy(APP.GrachtClient(), nullptr, Id());
    }
    
    void* MemoryPool::CreateBufferPointer(int memoryOffset)
    {
        uint8_t* bufferPointer = static_cast<uint8_t*>(m_attachment.buffer);
        if (bufferPointer == nullptr) {
            return nullptr;
        }
        
        if (memoryOffset >= m_size) {
            return nullptr;
        }
        
        return static_cast<void*>(bufferPointer + memoryOffset);
    }
}
