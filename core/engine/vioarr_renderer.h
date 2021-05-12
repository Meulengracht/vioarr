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
 
#ifndef __VIOARR_RENDERER_H__
#define __VIOARR_RENDERER_H__

#include "vioarr_screen.h"

typedef struct vioarr_renderer vioarr_renderer_t;

vioarr_renderer_t* vioarr_renderer_create(vioarr_screen_t*);
void               vioarr_renderer_set_scale(vioarr_renderer_t*, int);
void               vioarr_renderer_set_rotation(vioarr_renderer_t*, int);
int                vioarr_renderer_scale(vioarr_renderer_t*);
int                vioarr_renderer_rotation(vioarr_renderer_t*);

int                vioarr_renderer_create_image(vioarr_renderer_t*, vioarr_buffer_t*);
void               vioarr_renderer_destroy_image(vioarr_renderer_t*, int);

void               vioarr_renderer_wait_frame(vioarr_renderer_t*);
void               vioarr_renderer_render(vioarr_renderer_t*);

#endif //!__VIOARR_RENDERER_H__
