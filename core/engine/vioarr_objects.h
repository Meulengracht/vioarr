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

#ifndef __VIOARR_OBJECTS_H__
#define __VIOARR_OBJECTS_H__

#include <stdint.h>
#include "../protocols/wm_core_protocol.h"

void     vioarr_objects_create_client_object(int, uint32_t, void*, enum wm_core_object_type);
uint32_t vioarr_objects_create_server_object(void*, enum wm_core_object_type);
int      vioarr_objects_remove_object(int, uint32_t);
void     vioarr_objects_remove_by_client(int);
void*    vioarr_objects_get_object(int, uint32_t);
void     vioarr_objects_publish(int);

#endif //!__VIOARR_OBJECTS_H__
