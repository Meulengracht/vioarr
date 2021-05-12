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

#ifndef __VIOARR_MANAGER_H__
#define __VIOARR_MANAGER_H__

#include <os/osdefs.h>

typedef struct list list_t;
typedef struct vioarr_surface vioarr_surface_t;

/**
 * The number of surface 'levels' they can be drawn at. There is the bottom level,
 * default level, top level and cursor level.
 */
#define SURFACE_LEVELS 4

void              vioarr_manager_register_surface(vioarr_surface_t* surface);
void              vioarr_manager_unregister_surface(vioarr_surface_t* surface);
void              vioarr_manager_promote_cursor(vioarr_surface_t* surface);
void              vioarr_manager_demote_cursor(vioarr_surface_t* surface);
void              vioarr_manager_change_level(vioarr_surface_t* surface, int level);
void              vioarr_manager_render_start(list_t** surfaceLevels);
void              vioarr_manager_render_end(void);
vioarr_surface_t* vioarr_manager_front_surface(void);
vioarr_surface_t* vioarr_manager_surface_at(int x, int y, int* localX, int* localY);
void              vioarr_manager_focus_surface(vioarr_surface_t* surface);

#endif //!__VIOARR_MANAGER_H__
