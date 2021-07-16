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
#include <os/mollenos.h>
#include "../vioarr_manager.h"
#include "../vioarr_engine.h"
#include "../vioarr_renderer.h"
#include "../vioarr_screen.h"
#include "../vioarr_utils.h"
#include <time.h>
#include <threads.h>

struct startup_sync_context {
    mtx_t lock;
    cnd_t signal;
};

struct render_sync_context {
    mtx_t   lock;
    cnd_t   signal;
    clock_t last_update;
};

static int vioarr_engine_setup_screens(void);
static int vioarr_engine_update(void*);

static vioarr_screen_t*            primary_screen;
static thrd_t                      screen_thread;
static struct startup_sync_context startup_context;
static struct render_sync_context  render_sync;

int vioarr_engine_initialize(void)
{
    int status;
    
    // initialize systems
    vioarr_manager_initialize();
    
    // initialize the startup context that synchronizes
    // the startup sequence. 
    mtx_init(&startup_context.lock, mtx_plain);
    cnd_init(&startup_context.signal);

    // initialize the rendering sync context that controls
    // how often we render
    mtx_init(&render_sync.lock, mtx_plain);
    cnd_init(&render_sync.signal);
    render_sync.last_update = 0;

    // create the renderer thread and allow it to initialize before ending engine init
    vioarr_utils_trace(VISTR("[vioarr] [initialize] creating renderer thread"));
    status = thrd_create(&screen_thread, vioarr_engine_update, NULL);
    if (status != thrd_success) {
        return status;
    }

    // wait for startup sequence to finish
    mtx_lock(&startup_context.lock);
    cnd_wait(&startup_context.signal, &startup_context.lock);
    mtx_unlock(&startup_context.lock);
    return 0;
}


int vioarr_engine_x_minimum(void)
{
    return vioarr_region_x(vioarr_screen_region(primary_screen));
}

int vioarr_engine_x_maximum(void)
{
    return vioarr_region_width(vioarr_screen_region(primary_screen));
}

int vioarr_engine_y_minimum(void)
{
    return vioarr_region_y(vioarr_screen_region(primary_screen));
}

int vioarr_engine_y_maximum(void)
{
    return vioarr_region_height(vioarr_screen_region(primary_screen));
}

static int vioarr_engine_setup_screens(void)
{
    video_output_t video;
    OsStatus_t     osStatus;
    
    vioarr_utils_trace(VISTR("[vioarr] [initialize] quering screen information"));
    // Get screens available from OS.
    osStatus = QueryDisplayInformation(&video);
    if (osStatus != OsSuccess) {
        vioarr_utils_error(VISTR("[vioarr] [initialize] failed to query screens, status %u"), osStatus);
        OsStatusToErrno(osStatus);
        return -1;
    }
    
    vioarr_utils_trace(VISTR("[vioarr] [initialize] creating primary screen object"));
    // Create the primary screen object. In the future we will support
    // multiple displays and also listen for screen hotplugs
    primary_screen = vioarr_screen_create(&video);
    if (!primary_screen) {
        vioarr_utils_error(VISTR("[vioarr] [initialize] failed to create primary screen object"));
        return -1;
    }
    return 0;
}

static void signal_init_thread(void)
{
    mtx_lock(&startup_context.lock);
    cnd_signal(&startup_context.signal);
    mtx_unlock(&startup_context.lock);
}

static int vioarr_engine_update(void* context)
{
    clock_t update, diffMs;
    int     status;

    (void)context;

    vioarr_utils_trace(VISTR("vioarr_engine_update initializing screens"));
    status = vioarr_engine_setup_screens();
    if (status) {
        vioarr_utils_error(VISTR("vioarr_engine_update failed to initialize screens, code %i"), status);
        signal_init_thread();
        return status;
    }
    
    vioarr_utils_trace(VISTR("vioarr_engine_update started"));
    signal_init_thread();
    
    vioarr_utils_trace(VISTR("[vioarr] [renderer_thread] started"));
    while (1) {
        mtx_lock(&render_sync.lock);
        cnd_wait(&render_sync.signal, &render_sync.lock);
        mtx_unlock(&render_sync.lock);

        update = clock();
        diffMs = (update - render_sync.last_update) / (CLOCKS_PER_SEC / 1000);
        if (diffMs < ENGINE_SCREEN_REFRESH_MS) {
            thrd_sleepex(ENGINE_SCREEN_REFRESH_MS - (diffMs % ENGINE_SCREEN_REFRESH_MS));
        }
        render_sync.last_update = update;
        start = clock();
        
        vioarr_screen_frame(screen);
    }
}
