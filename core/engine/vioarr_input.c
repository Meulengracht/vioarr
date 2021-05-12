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

#include <ds/list.h>
#include <os/keycodes.h>
#include "vioarr_engine.h"
#include "vioarr_input.h"
#include "vioarr_objects.h"
#include "vioarr_manager.h"
#include "vioarr_region.h"
#include "vioarr_surface.h"
#include "vioarr_utils.h"
#include "../protocols/wm_core_protocol.h"
#include "../protocols/wm_surface_protocol_server.h"
#include "../protocols/wm_pointer_protocol_server.h"
#include "../protocols/wm_keyboard_protocol_server.h"
#include <stdlib.h>
#include <string.h>

#define POINTER_MODE_NORMAL    0
#define POINTER_MODE_RESIZING  1
#define POINTER_MODE_MOVING    2
#define POINTER_MODE_GRABBED   3

typedef struct vioarr_input_source {
    element_t header;
    uint32_t  id;
    UUId_t    deviceId;
    int       type;
    union {
        struct {
            int                  x;
            int                  y;
            int                  z;
            vioarr_surface_t*    surface;

            // move/resize/grab op
            int                  mode;
            enum wm_surface_edge edge;
            vioarr_surface_t*    op_surface;
        } pointer;
    } state;
} vioarr_input_source_t;

static list_t g_inputDevices = LIST_INIT;

void vioarr_input_register(UUId_t deviceId, int type)
{
    vioarr_input_source_t* source;
    uint32_t               sourceId;

    source = malloc(sizeof(vioarr_input_source_t));
    if (!source) {
        return;
    }

    memset(source, 0, sizeof(vioarr_input_source_t));

    sourceId = vioarr_objects_create_server_object(source, object_type_pointer);

    ELEMENT_INIT(&source->header, (uintptr_t)deviceId, source);
    source->id       = sourceId;
    source->deviceId = deviceId;
    source->type     = type;

    // set initial pointer state to middle of screen
    if (type == VIOARR_INPUT_POINTER) {
        source->state.pointer.x = vioarr_engine_x_maximum() >> 1;
        source->state.pointer.y = vioarr_engine_y_maximum() >> 1;
    }
    list_append(&g_inputDevices, &source->header);
}

void vioarr_input_unregister(UUId_t deviceId)
{
    vioarr_input_source_t* source = list_find_value(&g_inputDevices, (void*)(uintptr_t)deviceId);
    if (!source) {
        return;
    }

    list_remove(&g_inputDevices, &source->header);
    if (source->state.pointer.surface) {
        vioarr_manager_demote_cursor(source->state.pointer.surface);
    }

    // destroy server object
    vioarr_objects_remove_object(-1, source->id);
    free(source);
}

void vioarr_input_set_surface(vioarr_input_source_t* input, vioarr_surface_t* surface, int xOffset, int yOffset)
{
    if (!input) {
        return;
    }

    if (surface != NULL && input->state.pointer.surface == surface) {
        vioarr_surface_move_absolute(surface, 
            input->state.pointer.x + xOffset,
            input->state.pointer.y + yOffset);
        return;
    }

    if (input->state.pointer.surface) {
        // when clearing a surface for a pointer we will automatically hide it so
        // it doesn't float around.
        vioarr_manager_demote_cursor(input->state.pointer.surface);
        vioarr_surface_set_buffer(input->state.pointer.surface, NULL);
        vioarr_surface_commit(input->state.pointer.surface);
    }

    if (surface) {
        vioarr_manager_promote_cursor(surface);
        vioarr_surface_move_absolute(surface, 
            input->state.pointer.x + xOffset,
            input->state.pointer.y + yOffset);
    }

    // update the stored surface pointer
    input->state.pointer.surface = surface;
}

void vioarr_input_request_resize(vioarr_input_source_t* input, vioarr_surface_t* surface, enum wm_surface_edge edge)
{
    vioarr_utils_trace("vioarr_input_request_resize()");
    if (!input || !surface) {
        return;
    }

    if (input->state.pointer.mode != POINTER_MODE_NORMAL) {
        return;
    }

    if (vioarr_surface_maximized(surface) || 
        !vioarr_surface_contains(surface, input->state.pointer.x, input->state.pointer.y)) {
        return;
    }

    input->state.pointer.op_surface = surface;
    input->state.pointer.mode       = POINTER_MODE_RESIZING;
    input->state.pointer.edge       = edge;
}

void vioarr_input_request_move(vioarr_input_source_t* input, vioarr_surface_t* surface)
{
    vioarr_utils_trace("vioarr_input_request_move()");
    if (!input || !surface) {
        return;
    }

    if (input->state.pointer.mode != POINTER_MODE_NORMAL) {
        return;
    }

    if (vioarr_surface_maximized(surface) || 
        !vioarr_surface_contains(surface, input->state.pointer.x, input->state.pointer.y)) {
        return;
    }

    input->state.pointer.op_surface = surface;
    input->state.pointer.mode = POINTER_MODE_MOVING;    
}


void vioarr_input_grab(vioarr_input_source_t* input, vioarr_surface_t* surface)
{
    vioarr_region_t* region;

    if (!input || !surface) {
        return;
    }

    if (input->state.pointer.mode != POINTER_MODE_NORMAL) {
        return;
    }

    if (!vioarr_surface_contains(surface, input->state.pointer.x, input->state.pointer.y)) {
        return;
    }

    // set the location of the mouse to the center of the surface
    region = vioarr_surface_region(surface);

    input->state.pointer.op_surface = surface;
    input->state.pointer.mode = POINTER_MODE_GRABBED;
    input->state.pointer.x = vioarr_region_x(region) + (vioarr_region_width(region) >> 1);
    input->state.pointer.y = vioarr_region_y(region) + (vioarr_region_height(region) >> 1);
}

static void __clear_state(vioarr_input_source_t* input)
{
    input->state.pointer.mode = POINTER_MODE_NORMAL;
    input->state.pointer.op_surface = NULL;
}

void vioarr_input_ungrab(vioarr_input_source_t* input, vioarr_surface_t* surface)
{
    if (!input || !surface) {
        return;
    }

    if (input->state.pointer.mode != POINTER_MODE_GRABBED ||
        input->state.pointer.op_surface != surface) {
        return;
    }

    __clear_state(input);
}

void vioarr_input_on_surface_destroy(vioarr_surface_t* surface)
{
    foreach (element, &g_inputDevices) {
        vioarr_input_source_t* input = element->value;
        if (input->state.pointer.op_surface == surface) {
            __clear_state(input);
        }
    }
}

static void __normal_mode_motion(vioarr_input_source_t* source, int clampedX, int clampedY, int z)
{
    vioarr_surface_t* currentSurface = source->state.pointer.op_surface;
    vioarr_surface_t* surfaceAfterMove;
    int               sendUpdates;
    int               localY, localX;
    vioarr_utils_trace("__normal_mode_motion()");

    // ... we currently do not use z
    surfaceAfterMove = vioarr_manager_surface_at(source->state.pointer.x + clampedX, source->state.pointer.y + clampedY, &localX, &localY);
    sendUpdates      = vioarr_surface_supports_input(surfaceAfterMove, localX, localY);

    // update values before proceeding
    source->state.pointer.x += clampedX;
    source->state.pointer.y += clampedY;
    source->state.pointer.z += z;
    if (source->state.pointer.surface) {
        vioarr_surface_move(source->state.pointer.surface, clampedX, clampedY);
    }

    // send leave event if the current is not null and not surfaceAfterMove
    if (currentSurface != surfaceAfterMove) {
        if (currentSurface) {
            vioarr_utils_trace("__normal_mode_motion sending leave event to %i:%u",
                vioarr_surface_client(currentSurface), vioarr_surface_id(currentSurface));
            wm_pointer_event_leave_single(
                vioarr_surface_client(currentSurface),
                source->id,
                vioarr_surface_id(currentSurface));
        }
        vioarr_utils_trace("__normal_mode_motion next active surface %i:%u",
            vioarr_surface_client(surfaceAfterMove), vioarr_surface_id(surfaceAfterMove));

        // If the new surface does not support input, then we clear operating surface
        // and move on
        if (!sendUpdates) {
            source->state.pointer.op_surface = NULL;
            return;
        }
        source->state.pointer.op_surface = surfaceAfterMove;

        // send enter event
        vioarr_utils_trace("__normal_mode_motion sending enter event to %i:%u",
            vioarr_surface_client(surfaceAfterMove), vioarr_surface_id(surfaceAfterMove));
        wm_pointer_event_enter_single(
                vioarr_surface_client(surfaceAfterMove),
                source->id,
                vioarr_surface_id(surfaceAfterMove),
                localX, localY);
        sendUpdates = 0;
    }

    // send move event
    if (sendUpdates) {
        vioarr_utils_trace("__normal_mode_motion sending move event to %i:%u",
                vioarr_surface_client(surfaceAfterMove), vioarr_surface_id(surfaceAfterMove));
        
        // send move event with surface local coordinates
        wm_pointer_event_move_single(
            vioarr_surface_client(surfaceAfterMove),
            source->id,
            vioarr_surface_id(surfaceAfterMove),
            localX, localY);
    }
}

static void __resize_mode_motion(vioarr_input_source_t* source, int clampedX, int clampedY)
{
    vioarr_surface_t* currentSurface = source->state.pointer.op_surface;
    vioarr_region_t*  region = vioarr_surface_region(currentSurface);
    vioarr_utils_trace("__resize_mode_motion()");

    if (!clampedX && !clampedY) {
        return;
    }

    vioarr_surface_resize(currentSurface,
        vioarr_region_width(region) + clampedX,
        vioarr_region_height(region) + clampedY,
        source->state.pointer.edge);

    source->state.pointer.x += clampedX;
    source->state.pointer.y += clampedY;
    if (source->state.pointer.surface) {
        vioarr_surface_move(source->state.pointer.surface, clampedX, clampedY);
    }
}

static void __move_mode_motion(vioarr_input_source_t* source, int clampedX, int clampedY)
{
    vioarr_surface_t* currentSurface = source->state.pointer.op_surface;
    vioarr_utils_trace("__move_mode_motion()");

    if (!clampedX && !clampedY) {
        return;
    }

    vioarr_surface_move(currentSurface, clampedX, clampedY);
    source->state.pointer.x += clampedX;
    source->state.pointer.y += clampedY;
    if (source->state.pointer.surface) {
        vioarr_surface_move(source->state.pointer.surface, clampedX, clampedY);
    }
}

static void __grabbed_mode_motion(vioarr_input_source_t* source, int clampedX, int clampedY, int z)
{
    vioarr_surface_t* currentSurface = source->state.pointer.op_surface;
    vioarr_utils_trace("__grabbed_mode_motion()");

    // grabbed mode is a bit simpler, the mouse is bound to a surface and should never move
    // so we skip any forms of events except for the move event that should just send the relative
    // movements directly to the surface. In fact we do not even need to use the clamped X and Y, but
    // only keep those for simplicity

    wm_pointer_event_move_single(
        vioarr_surface_client(currentSurface),
        source->id,
        vioarr_surface_id(currentSurface),
        clampedX, clampedY);

    // still store changes made to z axis
    source->state.pointer.z += z;
}

void vioarr_input_axis_event(UUId_t deviceId, int x, int y, int z)
{
    vioarr_input_source_t* source = list_find_value(&g_inputDevices, (void*)(uintptr_t)deviceId);
    int                    clampedX = x;
    int                    clampedY = -y; // y is down
    vioarr_utils_trace("vioarr_input_axis_event(x=%i, y=%i)", x, y);
    
    if (!source) {
        return;
    }

    // clamp the axis values
    if (source->state.pointer.x + clampedX > vioarr_engine_x_maximum())      clampedX = vioarr_engine_x_maximum() - source->state.pointer.x;
    else if (source->state.pointer.x + clampedX < vioarr_engine_x_minimum()) clampedX = source->state.pointer.x;
    if (source->state.pointer.y + clampedY > vioarr_engine_y_maximum())      clampedY = vioarr_engine_y_maximum() - source->state.pointer.y;
    else if (source->state.pointer.y + clampedY < vioarr_engine_y_minimum()) clampedY = source->state.pointer.y;

    if (source->state.pointer.mode == POINTER_MODE_NORMAL) {
        __normal_mode_motion(source, clampedX, clampedY, z);
    }
    else if (source->state.pointer.mode == POINTER_MODE_RESIZING) {
        __resize_mode_motion(source, clampedX, clampedY);
    }
    else if (source->state.pointer.mode == POINTER_MODE_MOVING) {
        __move_mode_motion(source, clampedX, clampedY);
    }
    else if (source->state.pointer.mode == POINTER_MODE_GRABBED) {
        __grabbed_mode_motion(source, clampedX, clampedY, z);
    }
}

static void __normal_mode_click(vioarr_input_source_t* source, uint32_t button, int pressed)
{
    vioarr_surface_t* clickedSurface = source->state.pointer.op_surface;
    int               localX, localY;
    int               sendClick = 1;
    vioarr_utils_trace("__normal_mode_click()");

    // get the current surface, check against current active one
    if (!clickedSurface) {
        clickedSurface = vioarr_manager_surface_at(source->state.pointer.x, source->state.pointer.y, &localX, &localY);
        sendClick      = vioarr_surface_supports_input(clickedSurface, localX, localY);
        if (sendClick) {
            source->state.pointer.op_surface = clickedSurface;
            wm_pointer_event_enter_single(
                vioarr_surface_client(clickedSurface),
                source->id,
                vioarr_surface_id(clickedSurface),
                localX, localY);
        }
    }
    vioarr_manager_focus_surface(clickedSurface);

    if (sendClick) {
        vioarr_utils_trace("__normal_mode_click clicked on surface %i:%u",
            vioarr_surface_client(clickedSurface), vioarr_surface_id(clickedSurface));
        wm_pointer_event_click_single(
            vioarr_surface_client(clickedSurface),
            source->id, 
            vioarr_surface_id(clickedSurface), 
            button, pressed);
    }
}

static void vioarr_input_pointer_click(vioarr_input_source_t* source, uint32_t button, int pressed)
{
    vioarr_utils_trace("vioarr_input_pointer_click()");
    if (source->state.pointer.mode == POINTER_MODE_NORMAL ||
        source->state.pointer.mode == POINTER_MODE_GRABBED) {
        __normal_mode_click(source, button, pressed);
    }
    else {
        if (button == 0 /* LMB */ && !pressed) {
            __clear_state(source);
        }
    }
}

void vioarr_input_button_event(UUId_t deviceId, uint32_t keycode, uint32_t modifiers)
{
    vioarr_utils_trace("vioarr_input_button_event()");
    vioarr_input_source_t* source = list_find_value(&g_inputDevices, (void*)(uintptr_t)deviceId);
    if (!source) {
        // should probably handle this or something
        vioarr_utils_error("vioarr_input_button_event no input device found with deviceId %u", deviceId);
        return;
    }

    if (source->type == VIOARR_INPUT_POINTER) {
        uint32_t button = keycode - (uint32_t)VK_LBUTTON;
        vioarr_input_pointer_click(source, button, (modifiers & VK_MODIFIER_RELEASED) ? 0 : 1);
    }
    else {
        // keyboard
        vioarr_surface_t* currentSurface = vioarr_manager_front_surface();
        if (currentSurface) {
            vioarr_utils_trace("vioarr_input_button_event sending key to %i:%u",
                vioarr_surface_client(currentSurface), vioarr_surface_id(currentSurface));
            wm_keyboard_event_key_single(
                vioarr_surface_client(currentSurface),
                vioarr_surface_id(currentSurface),
                keycode, 
                modifiers);
        }
        else {
            vioarr_utils_error("vioarr_input_button_event no surface for key event");
        }
    }
}
