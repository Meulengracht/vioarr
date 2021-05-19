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

static int vioarr_engine_setup_screens(void);
static int vioarr_engine_update(void*);

static vioarr_screen_t* primary_screen;
static thrd_t           screen_thread;

int vioarr_engine_initialize(void)
{
    int status;

    // initialize systems
    vioarr_manager_initialize();
    
    // Initialise GLFW
    if (!glfwInit()) {
        vioarr_utils_error("Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    vioarr_utils_trace("[vioarr] [initialize] initializing screens");
    status = vioarr_engine_setup_screens();
    if (status) {
        vioarr_utils_error("[vioarr] [initialize] failed to initialize screens, code %i", status);
        return status;
    }
    
    // Spawn the renderer thread, this will update the screen at a 60 hz frequency
    // and handle all redrawing
    vioarr_utils_trace("[vioarr] [initialize] creating screen renderer thread");
    return thrd_create(&screen_thread, vioarr_engine_update, primary_screen);
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
    
    vioarr_utils_trace("vioarr_engine_setup_screens quering screen information");
    primary = glfwGetPrimaryMonitor();
    if (!primary) {
        vioarr_utils_error("vioarr_engine_setup_screens failed to get primary monitor information");
        return -1;
    }
    
    vioarr_utils_trace("[vioarr] [initialize] creating primary screen object");
    // Create the primary screen object. In the future we will support
    // multiple displays and also listen for screen hotplugs
    primary_screen = vioarr_screen_create(primary);
    if (!primary_screen) {
        vioarr_utils_error("[vioarr] [initialize] failed to create primary screen object");
        return -1;
    }
    return 0;
}

static int vioarr_engine_update(void* context)
{
    vioarr_screen_t* screen = context;
    
    vioarr_utils_trace("[vioarr] [renderer_thread] started");
    while (vioarr_screen_valid(screen)) {
        vioarr_screen_frame(screen);
        glfwPollEvents();
    }
    return 0;
}
