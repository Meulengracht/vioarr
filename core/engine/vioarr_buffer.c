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

#include "vioarr_buffer.h"
#include "vioarr_objects.h"
#include <stdlib.h>

typedef struct vioarr_buffer {
    vioarr_memory_pool_t* pool;
    uint32_t              id;
    _Atomic(int)          references;
    int                   width;
    int                   height;
    enum wm_pixel_format  format;
    void*                 data;
    unsigned int          flags;
} vioarr_buffer_t;

int vioarr_buffer_create(uint32_t id, vioarr_memory_pool_t* pool, int poolIndex,
    int width, int height, int stride, enum wm_pixel_format format,
    unsigned int flags, vioarr_buffer_t** bufferOut)
{
    vioarr_buffer_t* buffer;
    size_t           size;
    
    // id is optional
    if (!pool || !bufferOut) {
        return -1;
    }
    
    buffer = malloc(sizeof(vioarr_buffer_t));
    if (!buffer) {
        return -1;
    }
    
    buffer->id         = id;
    buffer->pool       = pool;
    buffer->references = ATOMIC_VAR_INIT(1);
    buffer->width      = width;
    buffer->height     = height;
    buffer->format     = format;
    buffer->flags      = flags;

    buffer->data = vioarr_memory_pool_data(pool, poolIndex, height * stride);
    if (!buffer->data) {
        free(buffer);
        return -1;
    }
    
    *bufferOut = buffer;
    return 0;
}

int vioarr_buffer_acquire(vioarr_buffer_t* buffer)
{
    int references;
    
    if (!buffer) {
        return -1;
    }
    
    references = atomic_fetch_add(&buffer->references, 1);
    if (!references) {
        // tried to acquire destroyed buffer
        return -1;
    }
    return 0;
}

int vioarr_buffer_destroy(vioarr_buffer_t* buffer)
{
    int references;
    
    if (!buffer) {
        return -1;
    }
    
    references = atomic_fetch_sub(&buffer->references, 1);
    if (references == 1) {
        // destroy logic
        free(buffer);
    }
    return 0;
}

uint32_t vioarr_buffer_id(vioarr_buffer_t* buffer)
{
    if (!buffer) {
        return 0;
    }
    return buffer->id;
}

int vioarr_buffer_width(vioarr_buffer_t* buffer)
{
    if (!buffer) {
        return 0;
    }
    return buffer->width;
}

int vioarr_buffer_height(vioarr_buffer_t* buffer)
{
    if (!buffer) {
        return 0;
    }
    return buffer->height;
}

void* vioarr_buffer_data(vioarr_buffer_t* buffer)
{
    if (!buffer) {
        return NULL;
    }
    return buffer->data;
}

enum wm_pixel_format vioarr_buffer_format(vioarr_buffer_t* buffer)
{
    if (!buffer) {
        return (enum wm_pixel_format)0;
    }
    return buffer->format;
}

int vioarr_buffer_flags(vioarr_buffer_t* buffer)
{
    if (!buffer) {
        return 0;
    }
    return buffer->flags;
}
