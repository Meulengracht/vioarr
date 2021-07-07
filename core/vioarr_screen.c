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

#include "wm_screen_service_server.h"
#include "wm_core_service_server.h"
#include "engine/vioarr_renderer.h"
#include "engine/vioarr_screen.h"
#include "engine/vioarr_surface.h"
#include "engine/vioarr_objects.h"
#include "engine/vioarr_manager.h"
#include "engine/vioarr_utils.h"
#include <gracht/server.h>
#include <errno.h>

#define SPAWN_COORDINATES_COUNT 6
static int g_spawnCoordinateX[SPAWN_COORDINATES_COUNT] = { 100, 200, 300, 100, 200, 300 };
static int g_spawnCoordinateY[SPAWN_COORDINATES_COUNT] = { 100, 100, 100, 200, 200, 200 };
static int g_spawnIndex = 0;

void wm_screen_get_properties_invocation(struct gracht_message* message, const uint32_t id)
{
    vioarr_utils_trace(VISTR("[wm_screen_get_properties_callback] client %i"), message->client);
    vioarr_screen_t* screen = vioarr_objects_get_object(-1, id);
    vioarr_region_t* region;
    
    if (!screen) {
        vioarr_utils_error("wm_screen_get_properties_callback: screen was not found");
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_screen: object does not exist");
        return;
    }
    
    region = vioarr_screen_region(screen);
    wm_screen_event_properties_single(vioarr_get_server_handle(), message->client,
        id, vioarr_region_x(region), vioarr_region_y(region),
        vioarr_screen_transform(screen), vioarr_screen_scale(screen));
}

void wm_screen_get_modes_invocation(struct gracht_message* message, const uint32_t id)
{
    vioarr_utils_trace(VISTR("[wm_screen_get_modes_callback] client %i"), message->client);
    vioarr_screen_t* screen = vioarr_objects_get_object(-1, id);
    int              status;
    if (!screen) {
        vioarr_utils_error(VISTR("wm_screen_get_modes_callback: screen was not found"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_screen: object does not exist");
        return;
    }
    
    status = vioarr_screen_publish_modes(screen, message->client);
    if (status) {
        vioarr_utils_error(VISTR("wm_screen_get_modes_callback: failed to publish modes"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, status, "wm_screen: failed to publish modes");
    }
}

void wm_screen_set_scale_invocation(struct gracht_message* message, const uint32_t id, const int scale)
{
    vioarr_utils_trace(VISTR("[wm_screen_set_scale_callback] client %i"), message->client);
    vioarr_screen_t* screen = vioarr_objects_get_object(-1, id);
    if (!screen) {
        vioarr_utils_error(VISTR("wm_screen_set_scale_callback: screen was not found"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_screen: object does not exist");
        return;
    }
    
    vioarr_screen_set_scale(screen, scale);
}

void wm_screen_set_transform_invocation(struct gracht_message* message, const uint32_t id, const enum wm_transform transform)
{
    vioarr_utils_trace(VISTR("[wm_screen_set_transform_callback] client %i"), message->client);
    vioarr_screen_t* screen = vioarr_objects_get_object(-1, id);
    if (!screen) {
        vioarr_utils_error(VISTR("wm_screen_set_transform_callback: screen was not found"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, id, ENOENT, "wm_screen: object does not exist");
        return;
    }
    
    vioarr_screen_set_transform(screen, transform);
}

void wm_screen_create_surface_invocation(struct gracht_message* message, 
    const uint32_t screenId, const uint32_t surfaceId, 
    const int x, const int y,
    const int width, const int height)
{
    vioarr_utils_trace(VISTR("[wm_screen_create_surface_callback] client %i"), message->client);
    vioarr_screen_t*   screen = vioarr_objects_get_object(-1, screenId);
    vioarr_surface_t*  surface;
    int                status;
    int                spawnX = x;
    int                spawnY = y;
    uint32_t           globalId;
    if (!screen) {
        vioarr_utils_error(VISTR("wm_screen_create_surface_callback: screen was not found"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, screenId, ENOENT, "wm_screen: object does not exist");
        return;
    }

    if (spawnX == -1) {
        spawnX = g_spawnCoordinateX[g_spawnIndex % SPAWN_COORDINATES_COUNT];
    }
    if (spawnY == -1) {
        spawnY = g_spawnCoordinateY[g_spawnIndex % SPAWN_COORDINATES_COUNT];
    }
    
    status = vioarr_surface_create(message->client, surfaceId, screen,
        spawnX, spawnY, width, height, &surface);
    if (status) {
        vioarr_utils_error(VISTR("wm_screen_create_surface_callback: failed to create surface"));
        wm_core_event_error_single(vioarr_get_server_handle(), message->client, screenId, status, "wm_screen: failed to create surface");
        return;
    }

    // go to next spawn
    g_spawnIndex++;

    // notify of new surface
    vioarr_manager_register_surface(surface);
    globalId = vioarr_objects_create_client_object(message->client, surfaceId, surface, WM_OBJECT_TYPE_SURFACE);
    wm_core_event_object_single(vioarr_get_server_handle(), message->client, 
        surfaceId,
        globalId,
        0,
        WM_OBJECT_TYPE_SURFACE
    );
}
