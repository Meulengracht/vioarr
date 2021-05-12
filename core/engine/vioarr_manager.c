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

#include <ds/list.h>
#include "vioarr_manager.h"
#include "vioarr_surface.h"
#include "vioarr_utils.h"
#include <stdlib.h>
#include <threads.h>

typedef struct vioarr_manager {
    list_t            surfaces[SURFACE_LEVELS];
    vioarr_rwlock_t   lock;
    vioarr_surface_t* focused;
} vioarr_manager_t;

static vioarr_manager_t g_manager = { { LIST_INIT, LIST_INIT, LIST_INIT, LIST_INIT }, RWLOCK_INIT, NULL };

static void* __surface_key(vioarr_surface_t* surface)
{
    uint32_t surfaceId = vioarr_surface_id(surface);
    surfaceId |= ((uint32_t)vioarr_surface_client(surface) << 16);
    return (void*)(uintptr_t)surfaceId;
}

void vioarr_manager_register_surface(vioarr_surface_t* surface)
{
    if (!surface) {
        ERROR("[vioarr_manager_register_surface] null parameters");
        return;
    }

    element_t* element = malloc(sizeof(element_t));
    if (!element) {
        ERROR("[vioarr_manager_register_surface] out of memory");
        return;
    }

    ELEMENT_INIT(element, __surface_key(surface), surface);
    
    vioarr_rwlock_w_lock(&g_manager.lock);
    list_append(&g_manager.surfaces[vioarr_surface_level(surface)], element);
    vioarr_rwlock_w_unlock(&g_manager.lock);
}

void vioarr_manager_unregister_surface(vioarr_surface_t* surface)
{
    element_t* element;
    int        level;

    if (!surface) {
        vioarr_utils_error("[vioarr_renderer_register_surface] null parameters");
        return;
    }

    level = vioarr_surface_level(surface);

    vioarr_rwlock_w_lock(&g_manager.lock);
    if (g_manager.focused == surface) {
        g_manager.focused = NULL;
    }

    element = list_find(&g_manager.surfaces[level], __surface_key(surface));
    if (element) {
        list_remove(&g_manager.surfaces[level], element);
    }
    vioarr_rwlock_w_unlock(&g_manager.lock);
}

static void __change_surface_level(vioarr_surface_t* surface, int level, int newLevel)
{
    element_t* element = list_find(&g_manager.surfaces[level], __surface_key(surface));
    if (element) {
        list_remove(&g_manager.surfaces[level], element);
        list_append(&g_manager.surfaces[newLevel], element);
        vioarr_surface_set_level(surface, newLevel);
    }
}

void vioarr_manager_promote_cursor(vioarr_surface_t* surface)
{
    int level = vioarr_surface_level(surface);
    if (level < 0) {
        return;
    }

    vioarr_rwlock_w_lock(&g_manager.lock);
    __change_surface_level(surface, level, SURFACE_LEVELS - 1);
    vioarr_rwlock_w_unlock(&g_manager.lock);
}

void vioarr_manager_demote_cursor(vioarr_surface_t* surface)
{
    vioarr_rwlock_w_lock(&g_manager.lock);
    __change_surface_level(surface, SURFACE_LEVELS - 1, 1);
    vioarr_rwlock_w_unlock(&g_manager.lock);
}

void vioarr_manager_change_level(vioarr_surface_t* surface, int level)
{
    int oldLevel = vioarr_surface_level(surface);
    if (oldLevel < 0) {
        return;
    }

    if (level >= 0 && level < (SURFACE_LEVELS - 1)) {
        vioarr_rwlock_w_lock(&g_manager.lock);
        __change_surface_level(surface, oldLevel, level);
        vioarr_rwlock_w_unlock(&g_manager.lock);
    }
}

void vioarr_manager_render_start(list_t** surfaceLevels)
{
    vioarr_rwlock_r_lock(&g_manager.lock);
    if (surfaceLevels) *surfaceLevels = &g_manager.surfaces[0];
}

void vioarr_manager_render_end(void)
{
    vioarr_rwlock_r_unlock(&g_manager.lock);
}

vioarr_surface_t* vioarr_manager_front_surface(void)
{
    vioarr_surface_t* front = NULL;
    int               level;

    vioarr_rwlock_r_lock(&g_manager.lock);
    front = g_manager.focused;
    if (!front) {
        for (level = SURFACE_LEVELS - 2; level >= 0; level--) {
            // back element is front
            element_t* element = list_back(&g_manager.surfaces[level]);
            if (element) {
                front = element->value;
                break;
            }
        }
    }
    vioarr_rwlock_r_unlock(&g_manager.lock);
    return front;
}

vioarr_surface_t* vioarr_manager_surface_at(int x, int y, int* localX, int* localY)
{
    vioarr_surface_t* surfaceAt = NULL;
    int               level;

    vioarr_rwlock_r_lock(&g_manager.lock);
    for (level = SURFACE_LEVELS - 2; level >= 0; level--) {
        foreach_reverse(i, &g_manager.surfaces[level]) {
            vioarr_surface_t* surface = vioarr_surface_at(i->value, x, y, localX, localY);
            if (surface) {
                surfaceAt = surface;
                goto exit;
            }
        }
    }

exit:
    vioarr_rwlock_r_unlock(&g_manager.lock);
    return surfaceAt;
}

void vioarr_manager_focus_surface(vioarr_surface_t* surface)
{
    vioarr_surface_t* entering = vioarr_surface_parent(surface, 1);
    vioarr_surface_t* leaving  = NULL;
    int               level    = vioarr_surface_level(entering);

    vioarr_rwlock_w_lock(&g_manager.lock);
    if (entering != g_manager.focused) {
        leaving = g_manager.focused;
        g_manager.focused = entering;

        if (g_manager.focused) {
            element_t* element = list_find(&g_manager.surfaces[level], __surface_key(entering));
            if (element) {
                list_remove(&g_manager.surfaces[level], element);
                list_append(&g_manager.surfaces[level], element);
            }
            else {
                g_manager.focused = NULL;
                entering = NULL;
            }
        }
    }
    else {
        entering = NULL;
    }
    vioarr_rwlock_w_unlock(&g_manager.lock);

    if (leaving) {
        vioarr_surface_focus(leaving, 0);
    }
    if (entering) {
        vioarr_surface_focus(entering, 1);
    }
}
