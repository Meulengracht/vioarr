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
#include "wm_pointer_service_server.h"
#include "engine/vioarr_objects.h"
#include "engine/vioarr_input.h"
#include "engine/vioarr_surface.h"
#include "engine/vioarr_utils.h"
#include <errno.h>

void wm_pointer_set_surface_invocation(struct gracht_message* message, const uint32_t pointerId, const uint32_t surfaceId, const int xOffset, const int yOffset)
{
    vioarr_utils_trace("[wm_pointer_set_surface_callback] client %i, pointer %u, surface %u",
        message->client, pointerId, surfaceId);
    vioarr_surface_t*      surface = vioarr_objects_get_object(message->client, surfaceId);
    vioarr_input_source_t* pointer = vioarr_objects_get_object(message->client, pointerId);
    
    if (!surface) {
        vioarr_utils_error("wm_pointer_set_surface_callback: surface did not exist");
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, surfaceId, ENOENT, "wm_surface: object does not exist");
        return;
    }
    if (!pointer) {
        vioarr_utils_error("wm_pointer_set_surface_callback: pointer did not exist");
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, surfaceId, ENOENT, "wm_pointer: object does not exist");
        return;
    }

    vioarr_input_set_surface(pointer, surface, xOffset, yOffset);
}

void wm_pointer_grab_invocation(struct gracht_message* message, const uint32_t pointerId, const uint32_t surfaceId)
{
    vioarr_utils_trace("[wm_pointer_grab_callback] client %i, pointer %u, surface %u",
        message->client, pointerId, surfaceId);
    vioarr_surface_t*      surface = vioarr_objects_get_object(message->client, surfaceId);
    vioarr_input_source_t* pointer = vioarr_objects_get_object(message->client, pointerId);
    
    if (!surface) {
        vioarr_utils_error("wm_pointer_grab_callback: surface did not exist");
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, surfaceId, ENOENT, "wm_surface: object does not exist");
        return;
    }
    if (!pointer) {
        vioarr_utils_error("wm_pointer_grab_callback: pointer did not exist");
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, surfaceId, ENOENT, "wm_pointer: object does not exist");
        return;
    }

    vioarr_input_grab(pointer, surface);
}

void wm_pointer_ungrab_invocation(struct gracht_message* message, const uint32_t pointerId, const uint32_t surfaceId)
{
    vioarr_utils_trace("[wm_pointer_ungrab_callback] client %i, pointer %u, surface %u",
        message->client, pointerId, surfaceId);
    vioarr_surface_t*      surface = vioarr_objects_get_object(message->client, surfaceId);
    vioarr_input_source_t* pointer = vioarr_objects_get_object(message->client, pointerId);
    
    if (!surface) {
        vioarr_utils_error("wm_pointer_ungrab_callback: surface did not exist");
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, surfaceId, ENOENT, "wm_surface: object does not exist");
        return;
    }
    if (!pointer) {
        vioarr_utils_error("wm_pointer_ungrab_callback: pointer did not exist");
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, surfaceId, ENOENT, "wm_pointer: object does not exist");
        return;
    }

    vioarr_input_ungrab(pointer, surface);
}
