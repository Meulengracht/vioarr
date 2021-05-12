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

#include "protocols/wm_surface_protocol_server.h"
#include "protocols/wm_core_protocol_server.h"
#include "engine/vioarr_input.h"
#include "engine/vioarr_surface.h"
#include "engine/vioarr_screen.h"
#include "engine/vioarr_objects.h"
#include "engine/vioarr_manager.h"
#include "engine/vioarr_utils.h"
#include <errno.h>

void wm_surface_get_formats_callback(struct gracht_recv_message* message, struct wm_surface_get_formats_args* input)
{
    ENTRY("wm_surface_get_formats_callback(client=%i, surface=%u)", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        vioarr_utils_error("wm_surface_get_formats_callback: failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    ERROR("[wm_surface_get_formats_callback] FIXME: STUB FUNCTION");

exit:
    EXIT("wm_surface_get_formats_callback");
}

void wm_surface_set_buffer_callback(struct gracht_recv_message* message, struct wm_surface_set_buffer_args* input)
{
    ENTRY("wm_surface_set_buffer_callback(client=%i, surface=%u)", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    vioarr_buffer_t*  buffer  = vioarr_objects_get_object(message->client, input->buffer_id);
    if (!surface) {
        vioarr_utils_error("wm_surface_set_buffer_callback: failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_set_buffer(surface, buffer);

exit:
    EXIT("wm_surface_set_buffer_callback");
}

void wm_surface_set_input_region_callback(struct gracht_recv_message* message, struct wm_surface_set_input_region_args* input)
{
    ENTRY("wm_surface_set_input_region_callback(client=%i, surface=%u)", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        vioarr_utils_error("wm_surface_set_input_region_callback: failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_set_input_region(surface, input->x, input->y, input->width, input->height);

exit:
    EXIT("wm_surface_set_input_region_callback");
}

void wm_surface_request_fullscreen_mode(struct gracht_recv_message* message, struct wm_surface_request_fullscreen_mode_args* input)
{
    ENTRY("wm_surface_request_fullscreen_mode(client %i, surface %u)", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        vioarr_utils_error("wm_surface_request_fullscreen_mode: failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    switch (input->mode) {
        case fs_mode_exit: {
            vioarr_manager_change_level(surface, 1);
            vioarr_surface_restore_size(surface);
        } break;

        case fs_mode_normal: {
            vioarr_surface_maximize(surface);
        } break;

        case fs_mode_full: {
            vioarr_manager_change_level(surface, 2);
            vioarr_surface_maximize(surface);
        } break;

        default: break;
    }

exit:
    EXIT("wm_surface_request_fullscreen_mode");
}

void wm_surface_request_level(struct gracht_recv_message* message, struct wm_surface_request_level_args* input)
{
    ENTRY("wm_surface_request_level(client %i, surface %u)", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        vioarr_utils_error("wm_surface_request_level failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_manager_change_level(surface, input->level);

exit:
    EXIT("wm_surface_request_level");
}

void wm_surface_request_frame_callback(struct gracht_recv_message* message, struct wm_surface_request_frame_args* input)
{
    ENTRY("wm_surface_request_frame_callback(client %i, surface %u)", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        vioarr_utils_error("wm_surface_request_frame_callback: failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_request_frame(surface);

exit:
    EXIT("wm_surface_request_frame_callback");
}

void wm_surface_invalidate_callback(struct gracht_recv_message* message, struct wm_surface_invalidate_args* input)
{
    ENTRY("wm_surface_invalidate_callback(client %i, surface %u)", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        vioarr_utils_error("wm_surface_invalidate_callback failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_invalidate(surface, input->x, input->y, input->width, input->height);

exit:
    EXIT("wm_surface_invalidate_callback");
}

void wm_surface_set_transparency_callback(struct gracht_recv_message* message, struct wm_surface_set_transparency_args* input)
{
    ENTRY("wm_surface_set_transparency_callback(client %i, surface %u)", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        vioarr_utils_error("wm_surface_set_transparency_callback: failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_set_transparency(surface, input->enable);

exit:
    EXIT("wm_surface_set_transparency_callback");
}

void wm_surface_set_drop_shadow_callback(struct gracht_recv_message* message, struct wm_surface_set_drop_shadow_args* input)
{
    ENTRY("wm_surface_set_drop_shadow_callback(client %i, surface %u)", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        vioarr_utils_error("wm_surface_set_drop_shadow_callback: failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_set_drop_shadow(surface, input->x, input->y, input->width, input->height);

exit:
    EXIT("wm_surface_set_drop_shadow_callback");
}

void wm_surface_add_subsurface_callback(struct gracht_recv_message* message, struct wm_surface_add_subsurface_args* input)
{
    ENTRY("wm_surface_add_subsurface_callback(client %i, surface %u)", message->client, input->parent_id);
    vioarr_surface_t* parent_surface = vioarr_objects_get_object(message->client, input->parent_id);
    vioarr_surface_t* child_surface  = vioarr_objects_get_object(message->client, input->child_id);
    int               status;
    if (!parent_surface) {
        vioarr_utils_error("wm_surface_add_subsurface_callback: failed to find parent surface");
        wm_core_event_error_single(message->client, input->parent_id, ENOENT, "wm_surface: parent object does not exist");
        goto exit;
    }
    
    if (!child_surface) {
        vioarr_utils_error("wm_surface_add_subsurface_callback: failed to find surface");
        wm_core_event_error_single(message->client, input->child_id, ENOENT, "wm_surface: child object does not exist");
        goto exit;
    }

    // unregister the surface
    vioarr_manager_unregister_surface(child_surface);
    
    status = vioarr_surface_add_child(parent_surface, child_surface, input->x, input->y);
    if (status) {
        wm_core_event_error_single(message->client, input->parent_id, status, "wm_surface: failed to add surface as a child");
    }

exit:
    EXIT("wm_surface_add_subsurface_callback");
}

void wm_surface_resize_subsurface_callback(struct gracht_recv_message* message, struct wm_surface_resize_subsurface_args* input)
{
    ENTRY("wm_surface_resize_subsurface_callback(client %i, surface %u)", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        vioarr_utils_error("wm_surface_resize_subsurface_callback failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_resize(surface, input->width, input->height, no_edges);

exit:
    EXIT("wm_surface_resize_subsurface_callback");
}

void wm_surface_move_subsurface_callback(struct gracht_recv_message* message, struct wm_surface_move_subsurface_args* input)
{
    ENTRY("wm_surface_move_subsurface_callback(client %i, surface %u)", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        vioarr_utils_error("wm_surface_move_subsurface_callback: failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_move_absolute(surface, input->x_in_parent, input->y_in_parent);

exit:
    EXIT("wm_surface_move_subsurface_callback");
}

void wm_surface_commit_callback(struct gracht_recv_message* message, struct wm_surface_commit_args* input)
{
    ENTRY("wm_surface_commit_callback(client %i, surface %u)", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        vioarr_utils_error("wm_surface_commit_callback: failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_commit(surface);

exit:
    EXIT("wm_surface_commit_callback");
}

void wm_surface_resize_callback(struct gracht_recv_message* message, struct wm_surface_resize_args* input)
{
    ENTRY("wm_surface_resize_callback(client %i, pointer %u, surface %u)", message->client, input->pointer_id, input->surface_id);
    vioarr_surface_t*      surface = vioarr_objects_get_object(message->client, input->surface_id);
    vioarr_input_source_t* pointer = vioarr_objects_get_object(message->client, input->pointer_id);
    
    if (!surface) {
        vioarr_utils_error("wm_surface_resize_callback: failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    if (!pointer) {
        vioarr_utils_error("wm_surface_resize_callback: failed to find pointer");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_pointer: object does not exist");
        goto exit;
    }
    
    vioarr_input_request_resize(pointer, surface, input->edges);

exit:
    EXIT("wm_surface_resize_callback");
}

void wm_surface_move_callback(struct gracht_recv_message* message, struct wm_surface_move_args* input)
{
    ENTRY("wm_surface_move_callback(client %i, pointer %u, surface %u)", message->client, input->pointer_id, input->surface_id);
    vioarr_surface_t*      surface = vioarr_objects_get_object(message->client, input->surface_id);
    vioarr_input_source_t* pointer = vioarr_objects_get_object(message->client, input->pointer_id);
    
    if (!surface) {
        vioarr_utils_error("wm_surface_move_callback: failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    if (!pointer) {
        vioarr_utils_error("wm_surface_move_callback: failed to find pointer");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_pointer: object does not exist");
        goto exit;
    }
    
    vioarr_input_request_move(pointer, surface);

exit:
    EXIT("wm_surface_move_callback");
}

void wm_surface_destroy_callback(struct gracht_recv_message* message, struct wm_surface_destroy_args* input)
{
    ENTRY("wm_surface_destroy_callback(client %i, surface %u)", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        vioarr_utils_error("wm_surface_destroy_callback: failed to find surface");
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        goto exit;
    }
    
    vioarr_surface_destroy(surface);

exit:
    EXIT("wm_surface_destroy_callback");
}
