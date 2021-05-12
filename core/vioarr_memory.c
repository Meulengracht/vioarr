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

#include "protocols/wm_core_protocol_server.h"
#include "protocols/wm_memory_protocol_server.h"
#include "protocols/wm_memory_pool_protocol_server.h"
#include "protocols/wm_buffer_protocol_server.h"
#include "engine/vioarr_memory.h"
#include "engine/vioarr_buffer.h"
#include "engine/vioarr_objects.h"
#include "engine/vioarr_utils.h"
#include <errno.h>

void wm_memory_create_pool_callback(struct gracht_recv_message* message, struct wm_memory_create_pool_args* input)
{
    vioarr_utils_trace("[wm_memory_create_pool_callback] client %i", message->client);
    vioarr_memory_pool_t* pool;
    int                   status;
    
    status = vioarr_memory_create_pool(message->client, input->pool_id, input->system_handle, input->size, &pool);
    if (status) {
        wm_core_event_error_single(message->client, input->pool_id, status, "wm_memory: failed to create memory pool");
        return;
    }
    
    vioarr_objects_create_client_object(message->client, input->pool_id, pool, object_type_memory_pool);
    wm_core_event_object_single(message->client, input->pool_id, vioarr_memory_pool_handle(pool), object_type_memory_pool);
}

void wm_memory_pool_create_buffer_callback(struct gracht_recv_message* message, struct wm_memory_pool_create_buffer_args* input)
{
    vioarr_utils_trace("[wm_memory_pool_create_buffer_callback] client %i", message->client);
    vioarr_memory_pool_t* pool = vioarr_objects_get_object(message->client, input->pool_id);
    vioarr_buffer_t*      buffer;
    int                   status;
    if (!pool) {
        vioarr_utils_error("wm_memory_pool_create_buffer_callback: pool not found");
        wm_core_event_error_single(message->client, input->pool_id, ENOENT, "wm_memory: object does not exist");
        return;
    }
    
    status = vioarr_buffer_create(input->buffer_id, pool, input->offset, 
        input->width, input->height, input->stride, input->format, 
        input->flags, &buffer);
    if (status) {
        vioarr_utils_error("wm_memory_pool_create_buffer_callback: failed to create memory buffer");
        wm_core_event_error_single(message->client, input->pool_id, status, "wm_memory: failed to create memory buffer");
        return;
    }
    
    vioarr_objects_create_client_object(message->client, input->buffer_id, buffer, object_type_buffer);
    wm_core_event_object_single(message->client, input->buffer_id, UUID_INVALID, object_type_buffer);
}

void wm_memory_pool_destroy_callback(struct gracht_recv_message* message, struct wm_memory_pool_destroy_args* input)
{
    vioarr_utils_trace("[wm_memory_pool_destroy_callback] client %i", message->client);
    vioarr_memory_pool_t* pool = vioarr_objects_get_object(message->client, input->pool_id);
    int                   status;
    if (!pool) {
        vioarr_utils_error("wm_memory_pool_destroy_callback: pool did not exist");
        wm_core_event_error_single(message->client, input->pool_id, ENOENT, "wm_memory: object does not exist");
        return;
    }
    
    vioarr_memory_destroy_pool(pool);
}

void wm_buffer_destroy_callback(struct gracht_recv_message* message, struct wm_buffer_destroy_args* input)
{
    vioarr_utils_trace("[wm_buffer_destroy_callback] client %i", message->client);
    vioarr_buffer_t* buffer = vioarr_objects_get_object(message->client, input->buffer_id);
    int              status;
    if (!buffer) {
        vioarr_utils_error("wm_buffer_destroy_callback: buffer did not exist");
        wm_core_event_error_single(message->client, input->buffer_id, ENOENT, "wm_memory: object does not exist");
        return;
    }
    
    vioarr_buffer_destroy(buffer);
}
