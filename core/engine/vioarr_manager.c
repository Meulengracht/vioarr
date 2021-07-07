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

#include <list.h>
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

static vioarr_manager_t g_manager;

static void* __surface_key(vioarr_surface_t* surface)
{
    uint32_t surfaceId = vioarr_surface_id(surface);
    surfaceId |= ((uint32_t)vioarr_surface_client(surface) << 16);
    return (void*)(uintptr_t)surfaceId;
}

static void __focus_top_surface(void)
{
    // We only care about mid-level surfaces which contains all
    // the regular windows.
    foreach_reverse(i, &g_manager.surfaces[1]) {
            if (vioarr_surface_visible(i->value)) {
                g_manager.focused = i->value;
                return;
            }
        }
    // none to focus :(
    g_manager.focused = NULL;
}

void vioarr_manager_initialize(void)
{
    int i;

    vioarr_rwlock_init(&g_manager.lock);
    for (i = 0; i < SURFACE_LEVELS; i++) {
        list_construct(&g_manager.surfaces[i]);
    }
    g_manager.focused = NULL;
}

void vioarr_manager_register_surface(vioarr_surface_t* surface)
{
    if (!surface) {
        vioarr_utils_error(VISTR("[vioarr_manager_register_surface] null parameters"));
        return;
    }

    element_t* element = malloc(sizeof(element_t));
    if (!element) {
        vioarr_utils_error(VISTR("[vioarr_manager_register_surface] out of memory"));
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
        vioarr_utils_error(VISTR("[vioarr_renderer_register_surface] null parameters"));
        return;
    }

    level = vioarr_surface_level(surface);

    vioarr_rwlock_w_lock(&g_manager.lock);
    element = list_find(&g_manager.surfaces[level], __surface_key(surface));
    if (element) {
        list_remove(&g_manager.surfaces[level], element);
    }

    if (g_manager.focused == surface) {
        __focus_top_surface();
    }
    vioarr_rwlock_w_unlock(&g_manager.lock);
}

static void __change_surface_level(vioarr_surface_t* surface, int level, int newLevel)
{
    element_t* element = list_find(&g_manager.surfaces[level], __surface_key(surface));
    if (element) {
        list_remove(&g_manager.surfaces[level], element);
        list_append(&g_manager.surfaces[newLevel], element);
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
    vioarr_surface_set_level(surface, SURFACE_LEVELS - 1);
}

void vioarr_manager_demote_cursor(vioarr_surface_t* surface)
{
    vioarr_rwlock_w_lock(&g_manager.lock);
    __change_surface_level(surface, SURFACE_LEVELS - 1, 1);
    vioarr_rwlock_w_unlock(&g_manager.lock);
    vioarr_surface_set_level(surface, 1);
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

vioarr_surface_t* vioarr_manager_get_focused(void)
{
    vioarr_surface_t* front;

    vioarr_rwlock_r_lock(&g_manager.lock);
    front = g_manager.focused;
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

/**
 * We need to handle a few cases, because a parent surface is focused
 * as long as one of its sub-surfaces are focused. This essentially means
 * we need to focus the actual clicked surface, but also focus all the way up
 */
void vioarr_manager_focus_surface(vioarr_surface_t* surface)
{
    vioarr_surface_t* entering = surface;
    vioarr_surface_t* leaving  = NULL;

    vioarr_rwlock_w_lock(&g_manager.lock);
    if (entering != g_manager.focused) {
        leaving = g_manager.focused;
        g_manager.focused = entering;

        if (g_manager.focused) {
            vioarr_surface_t* parent = vioarr_surface_parent(surface, 1);
            if (parent != vioarr_surface_parent(leaving, 1)) {
                int        level   = vioarr_surface_level(parent);
                element_t* element = list_find(&g_manager.surfaces[level], __surface_key(parent));
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

/**
 * Occurs when a surface has explicitly requested focus, but in order for
 * the focus to be granted, we must check some minor permissions. If the current
 * focused surface is owned by the requester, then we immediately allow this.
 */
void vioarr_manager_request_focus(int client, vioarr_surface_t* surface)
{
    vioarr_surface_t* currentFocus = vioarr_manager_get_focused();
    if (!currentFocus || currentFocus == surface) {
        return; // do not allow this
    }

    // lets get the elder parent of both
    int currentOrigin = vioarr_surface_client(currentFocus);
    if (currentOrigin != client) {
        return; // ok we can't allow this heresay
    }

    vioarr_manager_focus_surface(surface);
}

/**
 * Should be invoked by the renderer when a visibility of surfaces happen.
 * When surfaces are shown, we want to grant them focus, and when they are hidden
 * we need to update the focused surface (if they are focused).
 * This function should only be invoked on root surfaces, as we do not act on subsurfaces
 * in this case
 */
void vioarr_manager_on_surface_visiblity_change(vioarr_surface_t* surface, int visible)
{
    vioarr_rwlock_r_unlock(&g_manager.lock);
    if (visible) {
        vioarr_manager_focus_surface(surface);
    }
    else if (vioarr_surface_parent(g_manager.focused, 1) == surface) {
        vioarr_rwlock_w_lock(&g_manager.lock);
        __focus_top_surface();
        vioarr_rwlock_w_unlock(&g_manager.lock);
    }
    vioarr_rwlock_r_lock(&g_manager.lock);
}
