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

#include "vioarr_surface.h"
#include "vioarr_screen.h"
#include "vioarr_region.h"
#include "vioarr_renderer.h"
#include "vioarr_buffer.h"
#include "vioarr_objects.h"
#include "vioarr_manager.h"
#include "vioarr_input.h"
#include "vioarr_utils.h"
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>

#include "../protocols/wm_surface_protocol_server.h"
#include "../protocols/wm_buffer_protocol_server.h"

typedef struct vioarr_surface_properties {
    int corner_radius;
    int border_width;
    int border_color;
    int transparent;
    
    vioarr_region_t*       input_region;
    vioarr_region_t*       drop_shadow;
    struct vioarr_surface* children;
} vioarr_surface_properties_t;

typedef struct vioarr_surface_backbuffer {
    int              resource_id;
    vioarr_buffer_t* content;
} vioarr_surface_backbuffer_t;

typedef struct vioarr_surface {
    int              client;
    uint32_t         id;
    vioarr_screen_t* screen;
    int              visible;
    vioarr_rwlock_t  lock;
    atomic_int       frame_requested;
    int              level;

    vioarr_region_t*       dimensions;
    vioarr_region_t*       dimensions_original;
    struct vioarr_surface* parent;
    struct vioarr_surface* link;
    vioarr_region_t*       dirt;
    
    vioarr_surface_properties_t properties[2];

    int                         swap_backbuffers;
    int                         backbuffer_index;
    vioarr_surface_backbuffer_t backbuffers[2];
} vioarr_surface_t;

#define ACTIVE_PROPERTIES(surface)  surface->properties[0]
#define PENDING_PROPERTIES(surface) surface->properties[1]

#define ACTIVE_BACKBUFFER(surface)  surface->backbuffers[surface->backbuffer_index]
#define PENDING_BACKBUFFER(surface) surface->backbuffers[surface->backbuffer_index ^ 1]

static int  __initialize_surface_properties(vioarr_surface_properties_t* properties);
static void __cleanup_surface_properties(vioarr_surface_properties_t* properties);
static void __cleanup_surface_backbuffer(vioarr_screen_t* screen, vioarr_surface_backbuffer_t* backbuffer);
static void __swap_properties(vioarr_surface_t* surface);
static void __update_surface(vcontext_t* context, vioarr_surface_t* surface);
static void __refresh_content(vcontext_t* context, vioarr_surface_t* surface);
static void __render_drop_shadow(vcontext_t* context, vioarr_surface_t* surface);
static void __render_content(vcontext_t* context, vioarr_surface_t* surface);
static void __remove_child(vioarr_surface_t* surface, vioarr_surface_t* child);
static void __make_orphan(vioarr_surface_t* surface);

static inline vioarr_region_t* __get_correct_region(vioarr_surface_t* surface)
{
    if (surface->dimensions_original) {
        return surface->dimensions_original;
    }
    return surface->dimensions;
}

static inline vioarr_region_t* __get_active_region(vioarr_surface_t* surface)
{
    return surface->dimensions;
}

static inline void __restore_region(vioarr_surface_t* surface)
{
    if (surface->dimensions_original) {
        surface->dimensions = surface->dimensions_original;
        surface->dimensions_original = NULL;
    }
}

int vioarr_surface_create(int client, uint32_t id, vioarr_screen_t* screen, int x, int y,
    int width, int height, vioarr_surface_t** surfaceOut)
{
    vioarr_surface_t* surface;
    
    if (!screen) {
        return -1;
    }
    
    surface = malloc(sizeof(vioarr_surface_t));
    if (!surface) {
        return -1;
    }
    
    memset(surface, 0, sizeof(vioarr_surface_t));
    vioarr_rwlock_init(&surface->lock);
    surface->client  = client;
    surface->id      = id;
    surface->screen  = screen;
    surface->level   = 1;

    surface->dimensions = vioarr_region_create();
    if (!surface->dimensions) {
        free(surface);
        return -1;
    }
    vioarr_region_add(surface->dimensions, 0, 0, width, height);
    vioarr_region_set_position(surface->dimensions, x, y);

    surface->dirt = vioarr_region_create();
    if (!surface->dirt) {
        free(surface->dimensions);
        free(surface);
        return -1;
    }
    
    if (__initialize_surface_properties(&surface->properties[0]) ||
        __initialize_surface_properties(&surface->properties[1])) {
        vioarr_surface_destroy(surface);
        return -1;
    }
    
    *surfaceOut = surface;
    return 0;
}

void vioarr_surface_destroy(vioarr_surface_t* surface)
{
    vioarr_surface_t* itr;

    if (!surface) {
        return;
    }

    // cleanup any external systems. In this case it's
    // important that any cleanup calls must happen after we unregister
    // the surface so noone gets a pointer to this.
    vioarr_manager_unregister_surface(surface);
    vioarr_input_on_surface_destroy(surface);

    // restore any changes to dimension
    __restore_region(surface);

    // if this surface is a child, remove it from the parent
    if (surface->parent) {
        __remove_child(surface->parent, surface);
        __make_orphan(surface);
        vioarr_renderer_wait_frame(vioarr_screen_renderer(surface->screen));
    }

    // if we have children, go through them and promote them to regular surfaces
    itr = ACTIVE_PROPERTIES(surface).children;
    while (itr) {
        vioarr_surface_t* next = itr->link;
        __make_orphan(itr);
        itr = next;
    }

    __cleanup_surface_properties(&surface->properties[0]);
    __cleanup_surface_properties(&surface->properties[1]);
    __cleanup_surface_backbuffer(surface->screen, &surface->backbuffers[0]);
    __cleanup_surface_backbuffer(surface->screen, &surface->backbuffers[1]);

    free(surface->dirt);
    free(surface->dimensions);
    free(surface);
}

int vioarr_surface_add_child(vioarr_surface_t* parent, vioarr_surface_t* child, int x, int y)
{
    TRACE("[vioarr_surface_add_child]");
    if (!parent || !child) {
        return -1;
    }

    if (child->parent) {
        return -1;
    }
    
    // parent member is not accessed by the render thread
    // and can "freely" be set like this
    child->parent = parent;

    // the list that we keep updated is actually the one in pending properties
    // which means all list changes are performed there
    vioarr_rwlock_w_lock(&parent->lock);
    if (PENDING_PROPERTIES(parent).children == NULL) {
        PENDING_PROPERTIES(parent).children = child;
    }
    else {
        vioarr_surface_t* itr = PENDING_PROPERTIES(parent).children;
        while (itr->link) {
            itr = itr->link;
        }
        itr->link = child;
    }
    vioarr_rwlock_w_unlock(&parent->lock);

    // update child position
    vioarr_surface_set_position(child, x, y);
    return 0;
}

static void __make_orphan(vioarr_surface_t* surface)
{
    if (!surface) {
        return;
    }

    vioarr_rwlock_w_lock(&surface->lock);
    surface->link   = NULL;
    surface->parent = NULL;
    vioarr_rwlock_w_unlock(&surface->lock);
}

static void __remove_child(vioarr_surface_t* surface, vioarr_surface_t* child)
{
    vioarr_surface_t* itr;

    if (!surface || !child) {
        return;
    }

    // at this point we can't just remove the surface, we'll have to wait a frame
    // after removing this as parent
    vioarr_rwlock_w_lock(&surface->lock);
    itr = ACTIVE_PROPERTIES(surface).children;
    if (itr == child) {
        ACTIVE_PROPERTIES(surface).children = child->link;
    }
    else {
        while (itr->link != child) {
            itr = itr->link;
        }

        itr->link = child->link;
    }
    vioarr_rwlock_w_unlock(&surface->lock);
}

void vioarr_surface_set_buffer(vioarr_surface_t* surface, vioarr_buffer_t* content)
{
    int resourceId = -1;

    if (!surface) {
        return;
    }

    if (content) {
        vioarr_buffer_acquire(content);
        resourceId = vioarr_renderer_create_image(vioarr_screen_renderer(surface->screen), content);
        TRACE("[vioarr_surface_set_buffer] initialized new content %i 0x%llx", resourceId, content);
    }
    
    vioarr_rwlock_w_lock(&surface->lock);
    if (PENDING_BACKBUFFER(surface).content) {
        TRACE("[vioarr_surface_set_buffer] cleaning up previous %i 0x%llx",
            PENDING_BACKBUFFER(surface).resource_id,
            PENDING_BACKBUFFER(surface).content);
        vioarr_renderer_destroy_image(vioarr_screen_renderer(surface->screen), PENDING_BACKBUFFER(surface).resource_id);
        vioarr_buffer_destroy(PENDING_BACKBUFFER(surface).content);
    }

    PENDING_BACKBUFFER(surface).content = content;
    PENDING_BACKBUFFER(surface).resource_id = resourceId;

    surface->swap_backbuffers = 1;
    vioarr_rwlock_w_unlock(&surface->lock);
}

void vioarr_surface_set_position(vioarr_surface_t* surface, int x, int y)
{
    if (!surface) {
        return;
    }

    vioarr_rwlock_w_lock(&surface->lock);
    vioarr_region_set_position(__get_correct_region(surface), x, y);
    vioarr_rwlock_w_unlock(&surface->lock);
}

void vioarr_surface_set_drop_shadow(vioarr_surface_t* surface, int x, int y, int width, int height)
{
    if (!surface) {
        return;
    }
    
    vioarr_rwlock_w_lock(&surface->lock);
    vioarr_region_zero(PENDING_PROPERTIES(surface).drop_shadow);
    vioarr_region_add(PENDING_PROPERTIES(surface).drop_shadow, x, y, width, height);
    vioarr_rwlock_w_unlock(&surface->lock);
}

void vioarr_surface_set_input_region(vioarr_surface_t* surface, int x, int y, int width, int height)
{
    if (!surface) {
        return;
    }
    
    vioarr_rwlock_w_lock(&surface->lock);
    vioarr_region_zero(PENDING_PROPERTIES(surface).input_region);
    vioarr_region_add(PENDING_PROPERTIES(surface).input_region, x, y, width, height);
    vioarr_rwlock_w_unlock(&surface->lock);
}

void vioarr_surface_set_transparency(vioarr_surface_t* surface, int enable)
{
    if (!surface) {
        return;
    }

    vioarr_rwlock_w_lock(&surface->lock);
    PENDING_PROPERTIES(surface).transparent = enable;
    vioarr_rwlock_w_unlock(&surface->lock);
}

void vioarr_surface_set_level(vioarr_surface_t* surface, int level)
{
    if (!surface) {
        return;
    }


    vioarr_rwlock_w_lock(&surface->lock);
    surface->level = level;
    vioarr_rwlock_w_unlock(&surface->lock);
}

void vioarr_surface_request_frame(vioarr_surface_t* surface)
{
    if (!surface) {
        return;
    }

    atomic_store(&surface->frame_requested, 1);
}

void vioarr_surface_maximize(vioarr_surface_t* surface)
{
    vioarr_region_t* maximized;

    if (!surface) {
        return;
    }

    // only supported on the root surface
    if (surface->parent) {
        maximized = surface->parent->dimensions;
    }
    else {
        maximized = vioarr_screen_region(surface->screen);
    }

    vioarr_rwlock_w_lock(&surface->lock);
    surface->dimensions_original = surface->dimensions;
    surface->dimensions = maximized;
    vioarr_rwlock_w_unlock(&surface->lock);

    wm_surface_event_resize_single(surface->client, surface->id,
        vioarr_region_width(maximized), 
        vioarr_region_height(maximized),
        no_edges);
}

void vioarr_surface_restore_size(vioarr_surface_t* surface)
{
    if (!surface) {
        return;
    }

    vioarr_rwlock_w_lock(&surface->lock);
    __restore_region(surface);
    vioarr_rwlock_w_unlock(&surface->lock);

    wm_surface_event_resize_single(surface->client, surface->id,
        vioarr_region_width(surface->dimensions), 
        vioarr_region_height(surface->dimensions),
        no_edges);
}

int vioarr_surface_supports_input(vioarr_surface_t* surface, int x, int y)
{
    vioarr_surface_t* itr;
    int               inputSupported = 0;
    if (!surface || !surface->visible) {
        return 0;
    }

    vioarr_rwlock_r_lock(&surface->lock);
    inputSupported = vioarr_region_contains(ACTIVE_PROPERTIES(surface).input_region, x, y);
    vioarr_rwlock_r_unlock(&surface->lock);
    return inputSupported;
}

int vioarr_surface_contains(vioarr_surface_t* surface, int x, int y)
{
    int contained = 0;

    if (!surface) {
        return 0;
    }

    vioarr_rwlock_r_lock(&surface->lock);
    if (!surface->visible) {
        goto exit;
    }

    contained = vioarr_region_contains(__get_active_region(surface), x, y);

exit:
    vioarr_rwlock_r_unlock(&surface->lock);
    return contained;
}

vioarr_surface_t* vioarr_surface_at(vioarr_surface_t* surface, int x, int y, int* localX, int* localY)
{
    vioarr_surface_t* surfaceAt = NULL;
    vioarr_region_t*  region;
    vioarr_utils_trace("vioarr_surface_at(surface=%u, x=%i, y=%i)", vioarr_surface_id(surface), x, y);
    if (!surface) {
        return NULL;
    }

    vioarr_rwlock_r_lock(&surface->lock);
    region = __get_active_region(surface);
    if (vioarr_region_contains(region, x, y)) {
        vioarr_surface_t* itr = ACTIVE_PROPERTIES(surface).children;
        int               xInSurface = x - vioarr_region_x(region);
        int               yInSurface = y - vioarr_region_y(region);

        while (itr) {
            vioarr_surface_t* subAt = vioarr_surface_at(itr, 
                xInSurface, yInSurface,
                localX, localY);
            if (subAt) {
                surfaceAt = subAt;
                break;
            }
            itr = itr->link;
        }

        // if we did not find a subsurface, then set this one as the innermost at
        if (!surfaceAt) {
            *localX = xInSurface;
            *localY = yInSurface;
            surfaceAt = surface;
        }
    }
    vioarr_rwlock_r_unlock(&surface->lock);

    vioarr_utils_trace("vioarr_surface_at returns=%u", vioarr_surface_id(surfaceAt));
    return surfaceAt;
}

vioarr_surface_t* vioarr_surface_parent(vioarr_surface_t* surface, int upperMost)
{
    vioarr_surface_t* parent = surface;

    if (!surface) {
        return NULL;
    }

    if (upperMost) {
        while (parent->parent) {
            parent = parent->parent;
        }
    }
    return parent;
}

void vioarr_surface_invalidate(vioarr_surface_t* surface, int x, int y, int width, int height)
{
    if (!surface) {
        return;
    }
    
    vioarr_rwlock_w_lock(&surface->lock);
    vioarr_region_add(surface->dirt, x, y, width, height);
    vioarr_rwlock_w_unlock(&surface->lock);
}

void vioarr_surface_commit(vioarr_surface_t* surface)
{
    if (!surface) {
        return;
    }

    vioarr_rwlock_w_lock(&surface->lock);
    if (surface->swap_backbuffers) {
        surface->backbuffer_index ^= 1;
        surface->swap_backbuffers = 0;
    }
    __swap_properties(surface);
    
    // Determine other attributes about this surface. Is it visible?
    surface->visible = ACTIVE_BACKBUFFER(surface).content != NULL;
    vioarr_rwlock_w_unlock(&surface->lock);
}

void vioarr_surface_move(vioarr_surface_t* surface, int x, int y)
{
    vioarr_region_t* region;

    if (!surface) {
        return;
    }

    vioarr_rwlock_w_lock(&surface->lock);
    region = __get_correct_region(surface);

    vioarr_region_set_position(region,
        vioarr_region_x(region) + x,
        vioarr_region_y(region) + y);
    vioarr_rwlock_w_unlock(&surface->lock);
}

void vioarr_surface_move_absolute(vioarr_surface_t* surface, int x, int y)
{
    if (!surface) {
        return;
    }

    vioarr_rwlock_w_lock(&surface->lock);
    vioarr_region_set_position(__get_correct_region(surface), x, y);
    vioarr_rwlock_w_unlock(&surface->lock);
}

void vioarr_surface_resize(vioarr_surface_t* surface, int width, int height, enum wm_surface_edge edges)
{
    if (!surface) {
        return;
    }

    vioarr_rwlock_w_lock(&surface->lock);
    vioarr_region_set_size(__get_correct_region(surface), width, height);
    vioarr_rwlock_w_unlock(&surface->lock);

    wm_surface_event_resize_single(surface->client, surface->id, width, height, edges);
}

void vioarr_surface_focus(vioarr_surface_t* surface, int focus)
{
    if (!surface) {
        return;
    }

    wm_surface_event_focus_single(surface->client, surface->id, focus);
}

uint32_t vioarr_surface_id(vioarr_surface_t* surface)
{
    if (!surface) {
        return 0;
    }
    return surface->id;
}

int vioarr_surface_client(vioarr_surface_t* surface)
{
    if (!surface) {
        return -1;
    }
    return surface->client;
}

vioarr_screen_t* vioarr_surface_screen(vioarr_surface_t* surface)
{
    if (!surface) {
        return NULL;
    }
    return surface->screen;
}

vioarr_region_t* vioarr_surface_region(vioarr_surface_t* surface)
{
    if (!surface) {
        return NULL;
    }
    return __get_active_region(surface);
}

int vioarr_surface_maximized(vioarr_surface_t* surface)
{
    if (!surface) {
        return 0;
    }
    return surface->dimensions_original != NULL;
}

int vioarr_surface_level(vioarr_surface_t* surface)
{
    if (!surface) {
        return -1;
    }
    return surface->level;
}

void vioarr_surface_render(vcontext_t* context, vioarr_surface_t* surface)
{
    vioarr_region_t* region;

    if (!surface) {
        return;
    }

    //TRACE("[vioarr_surface_render] %u [%i, %i]", 
    //    surface->id, vioarr_region_x(surface->dimensions),
    //    vioarr_region_y(surface->dimensions));
    vioarr_rwlock_r_lock(&surface->lock);
    if (!surface->visible) {
        vioarr_rwlock_r_unlock(&surface->lock);
        return;
    }

    __update_surface(context, surface);
    region = __get_active_region(surface);
#ifdef VIOARR_BACKEND_NANOVG
    nvgSave(context);
    nvgTranslate(context, 
        (float)vioarr_region_x(region),
        (float)vioarr_region_y(region)
    );

    // handle transparency of the surface
    if (ACTIVE_PROPERTIES(surface).transparent) {
        nvgGlobalCompositeBlendFunc(context, NVG_SRC_ALPHA, NVG_ONE_MINUS_SRC_ALPHA);
    }
    else {
        nvgGlobalCompositeOperation(context, NVG_COPY);
    }
#endif

    if (ACTIVE_BACKBUFFER(surface).content) {
        //TRACE("[vioarr_surface_render] rendering content");
        if (!vioarr_region_is_zero(ACTIVE_PROPERTIES(surface).drop_shadow)) {
            __render_drop_shadow(context, surface);
        }
        __render_content(context, surface);
    }

    if (ACTIVE_PROPERTIES(surface).children) {
        vioarr_surface_t* child = ACTIVE_PROPERTIES(surface).children;
        while (child) {
            vioarr_surface_render(context, child);
            child = child->link;
        }
    }
    vioarr_rwlock_r_unlock(&surface->lock);

#ifdef VIOARR_BACKEND_NANOVG
    nvgRestore(context);
#endif
}

static void __update_surface(NVGcontext* context, vioarr_surface_t* surface)
{
    //TRACE("[__update_surface]");
    __refresh_content(context, surface);
    if (atomic_exchange(&surface->frame_requested, 0)) {
        wm_surface_event_frame_single(surface->client, surface->id);
    }
    //TRACE("[__update_surface] is visible: %i", surface->visible);
}

static void __refresh_content(NVGcontext* context, vioarr_surface_t* surface)
{
    if (!vioarr_region_is_zero(surface->dirt)) {
        vioarr_buffer_t* buffer     = ACTIVE_BACKBUFFER(surface).content;
        int              resourceId = ACTIVE_BACKBUFFER(surface).resource_id;
        if (buffer) {
#ifdef VIOARR_BACKEND_NANOVG
            nvgUpdateImage(context, resourceId, (const uint8_t*)vioarr_buffer_data(buffer));
#endif
            wm_buffer_event_release_single(surface->client, vioarr_buffer_id(buffer));
        }

        vioarr_region_zero(surface->dirt);
    }
}

static void __swap_properties(vioarr_surface_t* surface)
{
    //TRACE("[__swap_properties]");

    // handle basic properties
    ACTIVE_PROPERTIES(surface).border_width  = PENDING_PROPERTIES(surface).border_width;
    ACTIVE_PROPERTIES(surface).border_color  = PENDING_PROPERTIES(surface).border_color;
    ACTIVE_PROPERTIES(surface).corner_radius = PENDING_PROPERTIES(surface).corner_radius;
    ACTIVE_PROPERTIES(surface).transparent   = PENDING_PROPERTIES(surface).transparent;
    vioarr_region_copy(ACTIVE_PROPERTIES(surface).drop_shadow,  PENDING_PROPERTIES(surface).drop_shadow);
    vioarr_region_copy(ACTIVE_PROPERTIES(surface).input_region, PENDING_PROPERTIES(surface).input_region);
    
    if (PENDING_PROPERTIES(surface).children) {
        // append the new children
        if (ACTIVE_PROPERTIES(surface).children) {
            vioarr_surface_t* itr = ACTIVE_PROPERTIES(surface).children;
            while (itr->link) {
                itr = itr->link;
            }
            itr->link = PENDING_PROPERTIES(surface).children;
        }
        else {
            ACTIVE_PROPERTIES(surface).children = PENDING_PROPERTIES(surface).children;
        }
        PENDING_PROPERTIES(surface).children = NULL;
    }
}

static void __render_content(vcontext_t* context, vioarr_surface_t* surface)
{
    float    width        = (float)vioarr_region_width(surface->dimensions);
    float    height       = (float)vioarr_region_height(surface->dimensions);
#ifdef VIOARR_BACKEND_NANOVG
    NVGpaint stream_paint = nvgImagePattern(context, 0.0f, 0.0f, width, height, 0.0f, 
        ACTIVE_BACKBUFFER(surface).resource_id, 1.0f);
    nvgBeginPath(context);
    nvgRect(context, 0.0f, 0.0f, width, height);
    nvgFillPaint(context, stream_paint);
    nvgFill(context);
#endif
}

static void __render_drop_shadow(vcontext_t* context, vioarr_surface_t* surface)
{
    float    width        = (float)vioarr_region_width(surface->dimensions);
    float    height       = (float)vioarr_region_height(surface->dimensions);
#ifdef VIOARR_BACKEND_NANOVG
    if (!ACTIVE_PROPERTIES(surface).transparent) {
        nvgSave(context);
        nvgGlobalCompositeBlendFunc(context, NVG_SRC_ALPHA, NVG_ONE_MINUS_SRC_ALPHA);
    }

	NVGpaint shadow_paint = nvgBoxGradient(context, 0, 0 + 2.0f, width, height, 
	    ACTIVE_PROPERTIES(surface).corner_radius * 2, 10, nvgRGBA(0, 0, 0, 128), nvgRGBA(0, 0, 0, 0));
	nvgBeginPath(context);
	//nvgRect(context, -10, -10, width + 20, height + 30);
	//nvgRoundedRect(context, 0, 0, surface->width, surface->height, surface->properties.corner_radius);
	nvgRect(context, 
        (float)vioarr_region_x(ACTIVE_PROPERTIES(surface).drop_shadow),
        (float)vioarr_region_y(ACTIVE_PROPERTIES(surface).drop_shadow),
        width + (float)vioarr_region_width(ACTIVE_PROPERTIES(surface).drop_shadow),
        height + (float)vioarr_region_height(ACTIVE_PROPERTIES(surface).drop_shadow));
	nvgPathWinding(context, NVG_HOLE);
	nvgFillPaint(context, shadow_paint);
	nvgFill(context);

    if (!ACTIVE_PROPERTIES(surface).transparent) {
        nvgRestore(context);
    }
#endif
}

static int __initialize_surface_properties(vioarr_surface_properties_t* properties)
{
    properties->drop_shadow  = vioarr_region_create();
    properties->input_region = vioarr_region_create();
    if (!properties->drop_shadow || !properties->input_region)
        return -1;
    return 0;
}

static void __cleanup_surface_properties(vioarr_surface_properties_t* properties)
{
    if (properties->input_region) {
        free(properties->input_region);
    }

    if (properties->drop_shadow) {
        free(properties->drop_shadow);
    }
}

static void __cleanup_surface_backbuffer(vioarr_screen_t* screen, vioarr_surface_backbuffer_t* backbuffer)
{
    if (backbuffer->content) {
        vioarr_renderer_destroy_image(vioarr_screen_renderer(screen), backbuffer->resource_id);
        vioarr_buffer_destroy(backbuffer->content);
    }
}
