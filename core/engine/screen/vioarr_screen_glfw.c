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

//#define __TRACE

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW. Always include it before gl.h and glfw3.h, since it's a bit magic.
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glad.h>
#include "../vioarr_input.h"
#include "../vioarr_renderer.h"
#include "../vioarr_screen.h"
#include "../vioarr_utils.h"
#include "../vioarr_objects.h"
#include "../../protocols/wm_screen_protocol_server.h"
#include <stdlib.h>

typedef struct vioarr_screen {
    uint32_t           id;
    GLFWwindow*        context;
    vioarr_region_t*   dimensions;
    vioarr_renderer_t* renderer;

    int                first_mouse;
    double             last_x;
    double             last_y;
} vioarr_screen_t;

// callbacks
static void glfw_mouse_callback(GLFWwindow* window, double xpos, double ypos);
static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void glfw_mouse_key_callback(GLFWwindow* window, int button, int action, int mods);
static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

vioarr_screen_t* vioarr_screen_create(video_output_t* video)
{
    vioarr_screen_t* screen;
    int              x, y, width, height;

    // Initialise GLFW
    glewExperimental = true; // Needed for core profile
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

    screen = malloc(sizeof(vioarr_screen_t));
    if (!screen) {
        goto error;
    }
    memset(screen, 0, sizeof(vioarr_screen_t));

    // get the size of the monitor
    glfwGetMonitorWorkarea(video, &x, &y, &width, &height);

    vioarr_utils_trace("[vioarr] [screen] [create] creating os_mesa context, version 3.3");
    screen->context = glfwCreateWindow(width, height, "Vioarr Window Manager", video, NULL);
    if (!screen->context) {
        vioarr_utils_error("Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        goto error;
    }
    
    vioarr_utils_trace("[vioarr] [screen] [create] allocating screen resources");
    screen->dimensions = vioarr_region_create();
    if (!screen->dimensions) {
        goto error;
    }
    
    vioarr_region_add(screen->dimensions, 0, 0, width, height);
    
    glfwSetWindowUserPointer(screen->context, screen);
    glfwSetCursorPosCallback(screen->context, glfw_mouse_callback);
    glfwSetScrollCallback(screen->context, glfw_scroll_callback);
    glfwSetMouseButtonCallback(screen->context, glfw_mouse_key_callback);
    glfwSetKeyCallback(screen->context, glfw_key_callback);

    // tell GLFW to capture our mouse, we don't want it shown
    glfwSetInputMode(screen->context, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the newly created context as current for now. We must have one pretty quickly
    glfwMakeContextCurrent(screen->context); // Initialize GLEW
    glewExperimental=true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        vioarr_utils_error("Failed to initialize GLEW\n");
        goto error;
    }
    
    vioarr_utils_trace("[vioarr] [screen] [create] loading gl extensions");
    status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (!status) {
        vioarr_utils_error("[vioarr] [initialize] failed to load gl extensions, code %i", status);
        goto error;
    }
    
    vioarr_utils_trace("[vioarr] [screen] [create] initializing renderer");
    screen->renderer = vioarr_renderer_create(screen);
    if (!screen->renderer) {
        goto error;
    }
    
    screen->id = vioarr_objects_create_server_object(screen, object_type_screen);
    screen->first_mouse = 1;

    // create the default mouse and keyboard objects
    vioarr_input_register(1, VIOARR_INPUT_POINTER);
    vioarr_input_register(2, VIOARR_INPUT_KEYBOARD);

    return screen;

error:
    if (screen->context) {
        glfwDestroyWindow(screen->context);
    }

    if (screen->dimensions) {
        free(screen->dimensions);
    }

    if (screen) {
        free(screen);
    }
    glfwTerminate();
    return NULL;
}

void vioarr_screen_set_scale(vioarr_screen_t* screen, int scale)
{
    if (!screen) {
        return;
    }
    vioarr_renderer_set_scale(screen->renderer, scale);
}

void vioarr_screen_set_transform(vioarr_screen_t* screen, enum wm_screen_transform transform)
{
    if (!screen) {
        return;
    }
    TRACE("[vioarr_screen_set_transform] FIXME: STUB FUNCTION");
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
    return vioarr_renderer_scale(screen->renderer);
}

enum wm_screen_transform vioarr_screen_transform(vioarr_screen_t* screen)
{
    int rotation;
    
    if (!screen) {
        return no_transform;
    }
    
    rotation = vioarr_renderer_rotation(screen->renderer);
    return no_transform; // TODO
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
    return wm_screen_event_mode_single(client, screen->id,
        mode_current | mode_preferred,
        vioarr_region_width(screen->dimensions),
        vioarr_region_height(screen->dimensions), 60);
}

int vioarr_screen_valid(vioarr_screen_t* screen);
{
    return screen != NULL && glfwWindowShouldClose(screen->context) == 0;
}

void vioarr_screen_frame(vioarr_screen_t* screen)
{
    ENTRY("vioarr_screen_frame()");
    vioarr_renderer_render(screen->renderer);
    glfwSwapBuffers(screen->context);
    EXIT("vioarr_screen_frame");
}

static void glfw_mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    vioarr_screen_t* screen = glfwGetWindowUserPointer(window);
    if (screen->first_mouse)
    {
        screen->last_x = xpos;
        screen->last_y = ypos;
        screen->first_mouse = 0;
    }

    // reversed since y-coordinates go from bottom to top
    float xoffset = xpos - screen->last_x;
    float yoffset = screen->last_y - ypos;

    screen->last_x = xpos;
    screen->last_y = ypos;

    vioarr_input_axis_event(1, (int)xoffset, (int)yoffset, 0);    
}

static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // not used yet
    (void)window;
    (void)xoffset;
    (void)yoffset;
    //vioarr_input_axis_event(1, (int)xoffset, (int)yoffset, 0);
}

static void glfw_mouse_key_callback(GLFWwindow* window, int button, int action, int mods)
{
    vioarr_input_button_event(1, (uint32_t)button, (uint32_t)mods);
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    vioarr_input_button_event(2, (uint32_t)scancode, (uint32_t)mods);
}
