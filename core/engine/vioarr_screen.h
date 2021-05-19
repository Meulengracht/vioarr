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
 
#ifndef __VIOARR_SCREEN_H__
#define __VIOARR_SCREEN_H__

#ifdef MOLLENOS
#include <ddk/video.h>
typedef VideoDescriptor_t video_output_t;
#elif defined(__linux__)
typedef struct GLFWmonitor video_output_t;
#endif

#include "wm_screen_service.h"
#include "vioarr_region.h"
#include "vioarr_surface.h"

typedef struct vioarr_screen vioarr_screen_t;
typedef struct vioarr_renderer vioarr_renderer_t;

vioarr_screen_t*   vioarr_screen_create(video_output_t*);
void               vioarr_screen_set_scale(vioarr_screen_t*, int);
void               vioarr_screen_set_transform(vioarr_screen_t*, enum wm_transform);
vioarr_region_t*   vioarr_screen_region(vioarr_screen_t*);
int                vioarr_screen_scale(vioarr_screen_t*);
enum wm_transform  vioarr_screen_transform(vioarr_screen_t*);
vioarr_renderer_t* vioarr_screen_renderer(vioarr_screen_t*);
int                vioarr_screen_publish_modes(vioarr_screen_t*, int);
int                vioarr_screen_valid(vioarr_screen_t*);
void               vioarr_screen_frame(vioarr_screen_t*);

#endif //!__VIOARR_SCREEN_H__
