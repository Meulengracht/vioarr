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

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include <glad.h>
#include <GLFW/glfw3.h>

#include "../vioarr_manager.h"
#include "../vioarr_engine.h"
#include "../vioarr_renderer.h"
#include "../vioarr_screen.h"
#include "../vioarr_utils.h"
#include <time.h>

struct start_sync_context {
    mtx_t lock;
    cnd_t signal;
};

static int vioarr_engine_setup_screens(void);
static int vioarr_engine_update(void*);

static vioarr_screen_t*          primary_screen;
static thrd_t                    screen_thread;
static struct start_sync_context startup_context;

int vioarr_engine_initialize(void)
{
    int status;

    // initialize systems
    vioarr_manager_initialize();
    
    mtx_init(&startup_context.lock, mtx_plain);
    cnd_init(&startup_context.signal);

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
    // glfwGetMonitors
    struct GLFWmonitor* primary;
    
    vioarr_utils_trace(VISTR("vioarr_engine_setup_screens quering screen information"));
    primary = glfwGetPrimaryMonitor();
    if (!primary) {
        vioarr_utils_error(VISTR("vioarr_engine_setup_screens failed to get primary monitor information"));
        return -1;
    }
    
    vioarr_utils_trace(VISTR("[vioarr] [initialize] creating primary screen object"));
    // Create the primary screen object. In the future we will support
    // multiple displays and also listen for screen hotplugs
    primary_screen = vioarr_screen_create(primary);
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
    int status;
    (void)context;

    // Initialise GLFW
    if (!glfwInit()) {
        vioarr_utils_error(VISTR("Failed to initialize GLFW\n"));
        signal_init_thread();
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    vioarr_utils_trace(VISTR("vioarr_engine_update initializing screens"));
    status = vioarr_engine_setup_screens();
    if (status) {
        vioarr_utils_error(VISTR("vioarr_engine_update failed to initialize screens, code %i"), status);
        signal_init_thread();
        return status;
    }
    
    vioarr_utils_trace(VISTR("vioarr_engine_update started"));
    signal_init_thread();
    while (vioarr_screen_valid(primary_screen)) {
        vioarr_screen_frame(primary_screen);
        glfwPollEvents();
    }
    return 0;
}
