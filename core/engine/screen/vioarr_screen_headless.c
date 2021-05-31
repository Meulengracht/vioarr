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

#include <ddk/video.h>
#include <glad/glad.h>
#include <GL/osmesa.h>
#include "../vioarr_renderer.h"
#include "../vioarr_screen.h"
#include "../vioarr_utils.h"
#include "../vioarr_objects.h"
#include "../../protocols/wm_screen_protocol_server.h"
#include <stdlib.h>

// Define a screen at 640*320*32 for headless environment
#define SCREEN_DEPTH  32
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 320

typedef struct vioarr_screen {
    uint32_t           id;
    OSMesaContext      context;
    void*              backbuffer;
    size_t             backbuffer_size;
    int                depth_bits;
    vioarr_region_t*   dimensions;
    vioarr_renderer_t* renderer;
} vioarr_screen_t;

vioarr_screen_t* vioarr_screen_create(video_output_t* video)
{
    vioarr_screen_t* screen;
    int attributes[100], n = 0;
    int status;
    
    screen = malloc(sizeof(vioarr_screen_t));
    if (!screen) {
        return NULL;
    }

    attributes[n++] = OSMESA_FORMAT;
    attributes[n++] = OSMESA_RGBA;
    attributes[n++] = OSMESA_DEPTH_BITS;
    attributes[n++] = 24;
    attributes[n++] = OSMESA_STENCIL_BITS;
    attributes[n++] = 8;
    attributes[n++] = OSMESA_ACCUM_BITS;
    attributes[n++] = 0;
    attributes[n++] = OSMESA_PROFILE;
    attributes[n++] = OSMESA_CORE_PROFILE;
    attributes[n++] = OSMESA_CONTEXT_MAJOR_VERSION;
    attributes[n++] = 3;
    attributes[n++] = OSMESA_CONTEXT_MINOR_VERSION;
    attributes[n++] = 3;
    attributes[n++] = 0;
    
    vioarr_utils_trace(VISTR("[vioarr] [screen] [create] creating os_mesa context, version 3.3"));
    screen->context = OSMesaCreateContextAttribs(&attributes[0], NULL);
    if (!screen->context) {
        free(screen);
        return NULL;
    }
    
    vioarr_utils_trace(VISTR("[vioarr] [screen] [create] allocating screen resources"));
    screen->dimensions = vioarr_region_create();
    if (!screen->dimensions) {
        OSMesaDestroyContext(screen->context);
        free(screen);
        return NULL;
    }
    
    screen->depth_bits = SCREEN_DEPTH;
    vioarr_region_add(screen->dimensions, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    screen->backbuffer_size = SCREEN_WIDTH * SCREEN_HEIGHT * 4 * sizeof(GLubyte);
    screen->backbuffer      = aligned_alloc(32, screen->backbuffer_size);
    if (!screen->backbuffer) {
        OSMesaDestroyContext(screen->context);
        free(screen->dimensions);
        free(screen);
        return NULL;
    }
    
    status = OSMesaMakeCurrent(screen->context, screen->backbuffer, GL_UNSIGNED_BYTE,
        SCREEN_WIDTH, SCREEN_HEIGHT);
    if (status == GL_FALSE) {
        vioarr_utils_error(VISTR("[vioarr] [initialize] failed to set the os_mesa context"));
        OSMesaDestroyContext(screen->context);
        free(screen);
        return NULL;
    }
    
    vioarr_utils_trace(VISTR("[vioarr] [screen] [create] loading gl extensions"));
    status = gladLoadGLLoader((GLADloadproc)OSMesaGetProcAddress, 3, 3);
    if (!status) {
        OSMesaDestroyContext(screen->context);
        free(screen);
        vioarr_utils_error(VISTR("[vioarr] [initialize] failed to load gl extensions, code %i"), status);
        return NULL;
    }
    
    vioarr_utils_trace(VISTR("[vioarr] [screen] [create] initializing renderer"));
    screen->renderer = vioarr_renderer_create(screen);
    if (!screen->renderer) {
        OSMesaDestroyContext(screen->context);
        free(screen->backbuffer);
        free(screen->dimensions);
        free(screen);
        return NULL;
    }
    
    screen->id = vioarr_objects_create_server_object(screen, object_type_screen);
    return screen;
}

void vioarr_screen_set_scale(vioarr_screen_t* screen, int scale)
{
    // do nothing
}

void vioarr_screen_set_transform(vioarr_screen_t* screen, enum wm_screen_transform transform)
{
    // do nothing
}

vioarr_region_t* vioarr_screen_region(vioarr_screen_t* screen)
{
    if (!screen) {
        return NULL;
    }
    return screen->dimensions;
}

int vioarr_screen_scale(vioarr_screen_t* screen)
{
    if (!screen) {
        return 1;
    }
    return 1;
}

enum wm_screen_transform vioarr_screen_transform(vioarr_screen_t* screen)
{
    return no_transform;
}

vioarr_renderer_t* vioarr_screen_renderer(vioarr_screen_t* screen)
{
    if (!screen) {
        return NULL;
    }
    return screen->renderer;
}

int vioarr_screen_publish_modes(vioarr_screen_t* screen, int client)
{
    if (!screen) {
        return -1;
    }
    
    // One hardcoded format
    return wm_screen_event_mode_single(client, screen->id, mode_current | mode_preferred,
        SCREEN_WIDTH, SCREEN_HEIGHT, 60);
}

void vioarr_screen_frame(vioarr_screen_t* screen)
{
    vioarr_renderer_render(screen->renderer);
    // no present logic in headless
}
