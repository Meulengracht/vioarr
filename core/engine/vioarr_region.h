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

#ifndef __VIOARR_REGION_H__
#define __VIOARR_REGION_H__

typedef struct vioarr_region vioarr_region_t;

vioarr_region_t* vioarr_region_create(void);
void             vioarr_region_zero(vioarr_region_t*);
void             vioarr_region_copy(vioarr_region_t*, vioarr_region_t*);
void             vioarr_region_set_position(vioarr_region_t*, int x, int y);
void             vioarr_region_set_size(vioarr_region_t*, int width, int height);
void             vioarr_region_add(vioarr_region_t*, int x, int y, int width, int height);
int              vioarr_region_x(vioarr_region_t*);
int              vioarr_region_y(vioarr_region_t*);
int              vioarr_region_width(vioarr_region_t*);
int              vioarr_region_height(vioarr_region_t*);
int              vioarr_region_is_zero(vioarr_region_t*);
int              vioarr_region_contains(vioarr_region_t*, int x , int y);
int              vioarr_region_intersects(vioarr_region_t*, vioarr_region_t*);

#endif //!__VIOARR_REGION_H__
