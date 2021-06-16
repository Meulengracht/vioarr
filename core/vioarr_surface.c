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

//#define __TRACE

#include "wm_surface_service_server.h"
#include "wm_core_service_server.h"
#include "engine/vioarr_input.h"
#include "engine/vioarr_surface.h"
#include "engine/vioarr_screen.h"
#include "engine/vioarr_objects.h"
#include "engine/vioarr_manager.h"
#include "engine/vioarr_utils.h"
#include <errno.h>

void wm_surface_get_formats_invocation(struct gracht_message* message, const uint32_t id)
{
    ENTRY(VISTR("wm_surface_get_formats_callback(client=%i, surface=%u)"), message->client, id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, id);
    if (!surface) {
        vioarr_utils_error(VISTR("wm_surface_get_formats_callback: failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_utils_error(VISTR("[wm_surface_get_formats_callback] FIXME: STUB FUNCTION"));

exit:
    EXIT("wm_surface_get_formats_callback");
}

void wm_surface_set_buffer_invocation(struct gracht_message* message, const uint32_t surfaceId, const uint32_t bufferId)
{
    ENTRY(VISTR("wm_surface_set_buffer_callback(client=%i, surface=%u)"), message->client, surfaceId);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, surfaceId);
    vioarr_buffer_t*  buffer  = vioarr_objects_get_object(message->client, bufferId);
    if (!surface) {
        vioarr_utils_error(VISTR("wm_surface_set_buffer_callback: failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, surfaceId, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_set_buffer(surface, buffer);

exit:
    EXIT("wm_surface_set_buffer_callback");
}

void wm_surface_set_input_region_invocation(struct gracht_message* message, const uint32_t id, const int x, const int y, const int width, const int height)
{
    ENTRY(VISTR("wm_surface_set_input_region_callback(client=%i, surface=%u)"), message->client, id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, id);
    if (!surface) {
        vioarr_utils_error(VISTR("wm_surface_set_input_region_callback: failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_set_input_region(surface, x, y, width, height);

exit:
    EXIT("wm_surface_set_input_region_callback");
}

void wm_surface_request_fullscreen_mode_invocation(struct gracht_message* message, const uint32_t id, const enum wm_fullscreen_mode mode)
{
    ENTRY(VISTR("wm_surface_request_fullscreen_mode(client %i, surface %u)"), message->client, id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, id);
    if (!surface) {
        vioarr_utils_error(VISTR("wm_surface_request_fullscreen_mode: failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    switch (mode) {
        case WM_FULLSCREEN_MODE_EXIT: {
            vioarr_manager_change_level(surface, 1);
            vioarr_surface_restore_size(surface);
        } break;

        case WM_FULLSCREEN_MODE_NORMAL: {
            vioarr_surface_maximize(surface);
        } break;

        case WM_FULLSCREEN_MODE_FULL: {
            vioarr_manager_change_level(surface, 2);
            vioarr_surface_maximize(surface);
        } break;

        default: break;
    }

exit:
    EXIT("wm_surface_request_fullscreen_mode");
}

void wm_surface_request_level_invocation(struct gracht_message* message, const uint32_t id, const int level)
{
    ENTRY(VISTR("wm_surface_request_level(client %i, surface %u)"), message->client, id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, id);
    if (!surface) {
        vioarr_utils_error(VISTR("wm_surface_request_level failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_manager_change_level(surface, level);

exit:
    EXIT("wm_surface_request_level");
}

void wm_surface_request_frame_invocation(struct gracht_message* message, const uint32_t id)
{
    ENTRY(VISTR("wm_surface_request_frame_callback(client %i, surface %u)"), message->client, id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, id);
    if (!surface) {
        vioarr_utils_error(VISTR("wm_surface_request_frame_callback: failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_request_frame(surface);

exit:
    EXIT("wm_surface_request_frame_callback");
}

void wm_surface_invalidate_invocation(struct gracht_message* message, const uint32_t id, const int x, const int y, const int width, const int height)
{
    ENTRY(VISTR("wm_surface_invalidate_callback(client %i, surface %u)"), message->client, id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, id);
    if (!surface) {
        vioarr_utils_error(VISTR("wm_surface_invalidate_callback failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_invalidate(surface, x, y, width, height);

exit:
    EXIT("wm_surface_invalidate_callback");
}

void wm_surface_set_drop_shadow_invocation(struct gracht_message* message, const uint32_t id, const int x, const int y, const int width, const int height)
{
    ENTRY(VISTR("wm_surface_set_drop_shadow_callback(client %i, surface %u)"), message->client, id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, id);
    if (!surface) {
        vioarr_utils_error(VISTR("wm_surface_set_drop_shadow_callback: failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_set_drop_shadow(surface, x, y, width, height);

exit:
    EXIT("wm_surface_set_drop_shadow_callback");
}

void wm_surface_add_subsurface_invocation(struct gracht_message* message, const uint32_t parentId, const uint32_t childId, const int x, const int y)
{
    ENTRY(VISTR("wm_surface_add_subsurface_callback(client %i, surface %u)"), message->client, parentId);
    vioarr_surface_t* parent_surface = vioarr_objects_get_object(message->client, parentId);
    vioarr_surface_t* child_surface  = vioarr_objects_get_object(message->client, childId);
    int               status;
    if (!parent_surface) {
        vioarr_utils_error(VISTR("wm_surface_add_subsurface_callback: failed to find parent surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, parentId, ENOENT, "wm_surface: parent object does not exist");
        goto exit;
    }
    
    if (!child_surface) {
        vioarr_utils_error(VISTR("wm_surface_add_subsurface_callback: failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, childId, ENOENT, "wm_surface: child object does not exist");
        goto exit;
    }

    // unregister the surface
    vioarr_manager_unregister_surface(child_surface);
    
    status = vioarr_surface_add_child(parent_surface, child_surface, x, y);
    if (status) {
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, parentId, status, "wm_surface: failed to add surface as a child");
    }

exit:
    EXIT("wm_surface_add_subsurface_callback");
}

void wm_surface_resize_subsurface_invocation(struct gracht_message* message, const uint32_t id, const int width, const int height)
{
    ENTRY(VISTR("wm_surface_resize_subsurface_callback(client %i, surface %u)"), message->client, id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, id);
    if (!surface) {
        vioarr_utils_error(VISTR("wm_surface_resize_subsurface_callback failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_resize(surface, width, height, WM_SURFACE_EDGE_NO_EDGES);

exit:
    EXIT("wm_surface_resize_subsurface_callback");
}

void wm_surface_move_subsurface_invocation(struct gracht_message* message, const uint32_t id, const int x, const int y)
{
    ENTRY(VISTR("wm_surface_move_subsurface_callback(client %i, surface %u)"), message->client, id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, id);
    if (!surface) {
        vioarr_utils_error(VISTR("wm_surface_move_subsurface_callback: failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_move_absolute(surface, x, y);

exit:
    EXIT("wm_surface_move_subsurface_callback");
}

void wm_surface_commit_invocation(struct gracht_message* message, const uint32_t id)
{
    ENTRY(VISTR("wm_surface_commit_callback(client %i, surface %u)"), message->client, id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, id);
    if (!surface) {
        vioarr_utils_error(VISTR("wm_surface_commit_callback: failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_commit(surface);
    // register/unregister

exit:
    EXIT("wm_surface_commit_callback");
}

void wm_surface_resize_invocation(struct gracht_message* message, const uint32_t id, const uint32_t pointerId, const enum wm_surface_edge edges)
{
    ENTRY(VISTR("wm_surface_resize_callback(client %i, pointer %u, surface %u)"), message->client, pointerId, id);
    vioarr_surface_t*      surface = vioarr_objects_get_object(message->client, id);
    vioarr_input_source_t* pointer = vioarr_objects_get_object(message->client, pointerId);
    
    if (!surface) {
        vioarr_utils_error(VISTR("wm_surface_resize_callback: failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    if (!pointer) {
        vioarr_utils_error(VISTR("wm_surface_resize_callback: failed to find pointer"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_pointer: object does not exist");
        goto exit;
    }
    
    vioarr_input_request_resize(pointer, surface, edges);

exit:
    EXIT("wm_surface_resize_callback");
}

void wm_surface_move_invocation(struct gracht_message* message, const uint32_t id, const uint32_t pointerId)
{
    ENTRY(VISTR("wm_surface_move_callback(client %i, pointer %u, surface %u)"), message->client, pointerId, id);
    vioarr_surface_t*      surface = vioarr_objects_get_object(message->client, id);
    vioarr_input_source_t* pointer = vioarr_objects_get_object(message->client, pointerId);
    
    if (!surface) {
        vioarr_utils_error(VISTR("wm_surface_move_callback: failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    if (!pointer) {
        vioarr_utils_error(VISTR("wm_surface_move_callback: failed to find pointer"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_pointer: object does not exist");
        goto exit;
    }
    
    vioarr_input_request_move(pointer, surface);

exit:
    EXIT("wm_surface_move_callback");
}

void wm_surface_destroy_invocation(struct gracht_message* message, const uint32_t id)
{
    ENTRY(VISTR("wm_surface_destroy_callback(client %i, surface %u)"), message->client, id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, id);
    if (!surface) {
        vioarr_utils_error(VISTR("wm_surface_destroy_callback: failed to find surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_destroy(surface);

exit:
    EXIT("wm_surface_destroy_callback");
}
