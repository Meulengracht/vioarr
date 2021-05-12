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

#ifndef __VIOARR_BUFFER_H__
#define __VIOARR_BUFFER_H__

#include "vioarr_memory.h"
#include "../protocols/wm_memory_pool_protocol.h"

typedef struct vioarr_buffer vioarr_buffer_t;

int                  vioarr_buffer_create(uint32_t id, vioarr_memory_pool_t* pool, int poolIndex, int width, 
                                            int height, int stride, enum wm_pixel_format format, 
                                            unsigned int flags, vioarr_buffer_t** bufferOut);
int                  vioarr_buffer_acquire(vioarr_buffer_t*);
int                  vioarr_buffer_destroy(vioarr_buffer_t*);
uint32_t             vioarr_buffer_id(vioarr_buffer_t*);
int                  vioarr_buffer_width(vioarr_buffer_t*);
int                  vioarr_buffer_height(vioarr_buffer_t*);
void*                vioarr_buffer_data(vioarr_buffer_t*);
enum wm_pixel_format vioarr_buffer_format(vioarr_buffer_t*);
int                  vioarr_buffer_flags(vioarr_buffer_t*);

#endif //!__VIOARR_BUFFER_H__
