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

#include "wm_core_service_server.h"
#include "wm_memory_service_server.h"
#include "wm_memory_pool_service_server.h"
#include "wm_buffer_service_server.h"
#include "engine/vioarr_memory.h"
#include "engine/vioarr_buffer.h"
#include "engine/vioarr_objects.h"
#include "engine/vioarr_utils.h"
#include <errno.h>

void wm_memory_create_pool_invocation(struct gracht_message* message, const uint32_t poolId, const size_t handle, const int size)
{
    vioarr_utils_trace(VISTR("[wm_memory_create_pool_callback] client %i"), message->client);
    vioarr_memory_pool_t* pool;
    int                   status;
    uint32_t              globalId;
    
    status = vioarr_memory_create_pool(message->client, poolId, handle, size, &pool);
    if (status) {
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, poolId, status, "wm_memory: failed to create memory pool");
        return;
    }
    
    globalId = vioarr_objects_create_client_object(message->client, poolId, pool, WM_OBJECT_TYPE_MEMORY_POOL);
    wm_core_event_object_single(vioarr_get_server_handle(), message->client, 
        poolId,
        globalId,
        vioarr_memory_pool_handle(pool), 
        WM_OBJECT_TYPE_MEMORY_POOL
    );
}

void wm_memory_pool_create_buffer_invocation(struct gracht_message* message, const uint32_t poolId, const uint32_t bufferId, const int offset, const int width, const int height, const int stride, const enum wm_pixel_format format, const unsigned int flags)
{
    vioarr_utils_trace(VISTR("[wm_memory_pool_create_buffer_callback] client %i"), message->client);
    vioarr_memory_pool_t* pool = vioarr_objects_get_object(message->client, poolId);
    vioarr_buffer_t*      buffer;
    int                   status;
    uint32_t              globalId;
    if (!pool) {
        vioarr_utils_error(VISTR("wm_memory_pool_create_buffer_callback: pool not found"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, poolId, ENOENT, "wm_memory: object does not exist");
        return;
    }
    
    status = vioarr_buffer_create(bufferId, pool, offset, 
        width, height, stride, format, flags, &buffer);
    if (status) {
        vioarr_utils_error(VISTR("wm_memory_pool_create_buffer_callback: failed to create memory buffer"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, poolId, status, "wm_memory: failed to create memory buffer");
        return;
    }
    
    globalId = vioarr_objects_create_client_object(message->client, bufferId, buffer, WM_OBJECT_TYPE_BUFFER);
    wm_core_event_object_single(vioarr_get_server_handle(), message->client, 
        bufferId, 
        globalId,
        0,
        WM_OBJECT_TYPE_BUFFER
    );
}

void wm_memory_pool_destroy_invocation(struct gracht_message* message, const uint32_t id)
{
    vioarr_utils_trace(VISTR("[wm_memory_pool_destroy_callback] client %i"), message->client);
    vioarr_memory_pool_t* pool = vioarr_objects_get_object(message->client, id);
    if (!pool) {
        vioarr_utils_error(VISTR("wm_memory_pool_destroy_callback: pool did not exist"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_memory: object does not exist");
        return;
    }
    
    vioarr_memory_destroy_pool(pool);
}

void wm_buffer_destroy_invocation(struct gracht_message* message, const uint32_t id)
{
    vioarr_utils_trace(VISTR("[wm_buffer_destroy_callback] client %i"), message->client);
    vioarr_buffer_t* buffer = vioarr_objects_get_object(message->client, id);
    if (!buffer) {
        vioarr_utils_error(VISTR("wm_buffer_destroy_callback: buffer did not exist"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_memory: object does not exist");
        return;
    }
    
    vioarr_buffer_destroy(buffer);
}
