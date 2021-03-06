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
#include "object_manager.hpp"
#include "object.hpp"

#ifdef MOLLENOS
#include <os/dmabuf.h>
#elif defined(_WIN32)

#else
#include <sys/types.h>
#endif

namespace Asgaard {
    class MemoryPool : public Object {
    public:
        ASGAARD_API MemoryPool(uint32_t id, std::size_t handle, std::size_t size);
        ASGAARD_API MemoryPool(uint32_t id, std::size_t size);
        ASGAARD_API ~MemoryPool();
        
        std::size_t Handle() const;
        std::size_t Size() const { return m_size; }
        bool        IsInheritted() const { return m_inheritted; }

    public:
        static std::shared_ptr<MemoryPool> Create(Object* owner, std::size_t size)
        {
            // Create the memory pool we're going to use
            auto memory = OM.CreateClientObject<MemoryPool, std::size_t>(size);
            memory->Subscribe(owner);
            return memory;
        }

        static std::shared_ptr<MemoryPool> Inherit(Object* owner, std::size_t handle, std::size_t size)
        {
            // Create the memory pool we're going to use
            auto memory = OM.CreateClientObject<MemoryPool, std::size_t, std::size_t>(handle, size);
            memory->Subscribe(owner);
            return memory;
        }
        
    public:
        void* CreateBufferPointer(int memoryOffset);
        
    private:
        std::size_t           m_size;
        bool                  m_inheritted;
        
#ifdef MOLLENOS
        struct dma_attachment m_attachment;
#elif defined(_WIN32)

#else
    key_t                     m_poolKey;
    int                       m_shmFd;
    void*                     m_memory;
#endif
    };
}
