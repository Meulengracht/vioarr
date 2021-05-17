/* MollenOS
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
 * Vioarr - Vali Compositor
 * - Implements the default system compositor for Vali. It utilizies the gracht library
 *   for communication between compositor clients and the server. The server renders
 *   using Mesa3D with either the soft-renderer or llvmpipe render for improved performance.
 */

#ifndef __VIOARR_MEMORY_H__
#define __VIOARR_MEMORY_H__

#if defined(MOLLENOS)
#include <os/osdefs.h>
typedef UUId_t mhandle_t;
#elif defined(_WIN32)


#elif defined(__linux__)
#include <sys/types.h>
#include <stdint.h>
typedef key_t mhandle_t;
#endif

typedef struct vioarr_memory_pool vioarr_memory_pool_t;

int       vioarr_memory_create_pool(int, uint32_t, mhandle_t, size_t, vioarr_memory_pool_t**);
int       vioarr_memory_pool_acquire(vioarr_memory_pool_t*);
int       vioarr_memory_destroy_pool(vioarr_memory_pool_t*);
uint32_t  vioarr_memory_pool_id(vioarr_memory_pool_t*);
mhandle_t vioarr_memory_pool_handle(vioarr_memory_pool_t*);
void*     vioarr_memory_pool_data(vioarr_memory_pool_t*, int, size_t);

#endif //!__VIOARR_MEMORY_H__
