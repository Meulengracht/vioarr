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
 
#ifndef __VIOARR_INPUT_H__
#define __VIOARR_INPUT_H__

#include <os/osdefs.h>

#define VIOARR_INPUT_POINTER  0
#define VIOARR_INPUT_KEYBOARD 1

typedef struct vioarr_surface      vioarr_surface_t;
typedef struct vioarr_input_source vioarr_input_source_t;
enum wm_surface_edge;

void vioarr_input_register(UUId_t deviceId, int);
void vioarr_input_unregister(UUId_t deviceId);
void vioarr_input_on_surface_destroy(vioarr_surface_t* surface);
void vioarr_input_axis_event(UUId_t deviceId, int x, int y, int z);
void vioarr_input_button_event(UUId_t deviceId, uint32_t keycode, uint32_t modifiers);

void vioarr_input_request_resize(vioarr_input_source_t* input, vioarr_surface_t* surface, enum wm_surface_edge);
void vioarr_input_request_move(vioarr_input_source_t* input, vioarr_surface_t* surface);
void vioarr_input_set_surface(vioarr_input_source_t* input, vioarr_surface_t* surface, int xOffset, int yOffset);
void vioarr_input_grab(vioarr_input_source_t* input, vioarr_surface_t* surface);
void vioarr_input_ungrab(vioarr_input_source_t* input, vioarr_surface_t* surface);

#endif //!__VIOARR_INPUT_H__
