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

#include "wm_core_service_client.h"
#include "wm_memory_pool_service_client.h"
#include "wm_memory_service_client.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

namespace Asgaard {
    MemoryPool::MemoryPool(uint32_t id, std::size_t handle, std::size_t size)
        : Object(id)
        , m_size(size)
        , m_inheritted(true)
        , m_poolKey(static_cast<key_t>(handle))
    {
        if (size == 0) {
            throw InvalidArgumentException("MemoryPool::MemoryPool 0-size provided");
        }

        m_shmFd = shmget(m_poolKey, m_size, S_IRUSR | S_IWUSR);
        if (m_shmFd == -1) {
            throw ApplicationException("MemoryPool::MemoryPool() failed to inherit memory pool", errno);
        }

        m_memory = shmat(m_shmFd, 0, 0);
        if (m_memory == (void*)(-1)) {
            throw ApplicationException("MemoryPool::MemoryPool() failed to inherit memory pool", errno);
        }
    }

    MemoryPool::MemoryPool(uint32_t id, std::size_t size)
        : Object(id)
        , m_size(size)
        , m_inheritted(false)
    {
        if (size == 0) {
            throw InvalidArgumentException("MemoryPool::MemoryPool 0-size provided");
        }

        // generate a new shmem key
        m_poolKey = ((getpid() & 0xFFFF) << 16) | (id & 0xFFFF); 

        m_shmFd = shmget(m_poolKey, size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
        if (m_shmFd == -1) {
            throw ApplicationException("MemoryPool::MemoryPool() failed to create a new memory pool", errno);
        }

        m_memory = shmat(m_shmFd, 0, 0);
        if (m_memory == (void*)(-1)) {
            throw ApplicationException("MemoryPool::MemoryPool() failed to map the new memory pool", errno);
        }

        wm_memory_create_pool(APP.GrachtClient(), nullptr, id, m_poolKey, size);
    }
    
    MemoryPool::~MemoryPool()
    {
        if (m_memory) {
            // destruction is handled by wm server
            shmdt(m_memory);
        }
        wm_memory_pool_destroy(APP.GrachtClient(), nullptr, Id());
    }
    
    void* MemoryPool::CreateBufferPointer(int memoryOffset)
    {
        uint8_t* bufferPointer = static_cast<uint8_t*>(m_memory);
        if (bufferPointer == nullptr) {
            return nullptr;
        }
        
        if ((size_t)memoryOffset >= m_size) {
            return nullptr;
        }
        
        return static_cast<void*>(bufferPointer + memoryOffset);
    }
}
