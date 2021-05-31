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

#include <glad.h>
#include <GLFW/glfw3.h>
#include "../vioarr_input.h"
#include "../vioarr_renderer.h"
#include "../vioarr_screen.h"
#include "../vioarr_utils.h"
#include "../vioarr_objects.h"
#include "wm_screen_service_server.h"
#include <stdlib.h>
#include <keycodes.h>

typedef struct vioarr_screen {
    uint32_t           id;
    GLFWwindow*        context;
    GLFWmonitor*       monitor;
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
    int              x, y, width, height, status;

    screen = malloc(sizeof(vioarr_screen_t));
    if (!screen) {
        goto error;
    }
    memset(screen, 0, sizeof(vioarr_screen_t));

    // get the size of the monitor
    glfwGetMonitorWorkarea(video, &x, &y, &width, &height);

    vioarr_utils_trace(VISTR("[vioarr] [screen] [create] creating os_mesa context, version 3.3"));
    screen->context = glfwCreateWindow(width, height, "Vioarr Window Manager", video, NULL);
    if (!screen->context) {
        vioarr_utils_error(VISTR("Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version.\n"));
        goto error;
    }
    
    vioarr_utils_trace(VISTR("[vioarr] [screen] [create] allocating screen resources"));
    screen->dimensions = vioarr_region_create();
    if (!screen->dimensions) {
        goto error;
    }
    
    glfwGetFramebufferSize(screen->context, &width, &height);
    vioarr_region_add(screen->dimensions, 0, 0, width, height);
    
    glfwSetWindowUserPointer(screen->context, screen);
    glfwSetCursorPosCallback(screen->context, glfw_mouse_callback);
    glfwSetScrollCallback(screen->context, glfw_scroll_callback);
    glfwSetMouseButtonCallback(screen->context, glfw_mouse_key_callback);
    glfwSetKeyCallback(screen->context, glfw_key_callback);

    // tell GLFW to capture our mouse, we don't want it shown
    glfwSetInputMode(screen->context, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the newly created context as current for now. We must have one pretty quickly
    glfwMakeContextCurrent(screen->context);
    
    vioarr_utils_trace(VISTR("[vioarr] [screen] [create] loading gl extensions"));
    status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress, 0, 0);
    if (!status) {
        vioarr_utils_error(VISTR("[vioarr] [initialize] failed to load gl extensions, code %i"), status);
        goto error;
    }
    
    vioarr_utils_trace(VISTR("[vioarr] [screen] [create] initializing renderer"));
    screen->renderer = vioarr_renderer_create(screen);
    if (!screen->renderer) {
        goto error;
    }
    
    screen->id = vioarr_objects_create_server_object(screen, WM_OBJECT_TYPE_SCREEN);
    screen->first_mouse = 1;
    screen->monitor = video;

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

void vioarr_screen_set_transform(vioarr_screen_t* screen, enum wm_transform transform)
{
    if (!screen) {
        return;
    }

    vioarr_utils_trace(VISTR("[vioarr_screen_set_transform] FIXME: STUB FUNCTION"));
    (void)transform;
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

enum wm_transform vioarr_screen_transform(vioarr_screen_t* screen)
{
    int rotation;
    
    if (!screen) {
        return WM_TRANSFORM_NO_TRANSFORM;
    }
    
    rotation = vioarr_renderer_rotation(screen->renderer);
    return WM_TRANSFORM_NO_TRANSFORM; // TODO
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
    const GLFWvidmode* modes;
    const GLFWvidmode* currentMode;
    int                modeCount;
    int                i;

    if (!screen) {
        return -1;
    }

    currentMode = glfwGetVideoMode(screen->monitor);
    modes       = glfwGetVideoModes(screen->monitor, &modeCount);
    if (!modes || !currentMode) {
        // One hardcoded format
        return wm_screen_event_mode_single(vioarr_get_server_handle(), client, screen->id, 
            WM_MODE_ATTRIBUTES_CURRENT | WM_MODE_ATTRIBUTES_PREFERRED,
            vioarr_region_width(screen->dimensions),
            vioarr_region_height(screen->dimensions), 
            60
        );
    }
    
    for (i = 0; i < modeCount; i++) {
        unsigned int attribs = 0;
        if (modes[i].width  == currentMode->width &&
            modes[i].height == currentMode->height &&
            modes[i].refreshRate == currentMode->refreshRate) {
            attribs = WM_MODE_ATTRIBUTES_CURRENT | WM_MODE_ATTRIBUTES_PREFERRED;
        }

        wm_screen_event_mode_single(vioarr_get_server_handle(), client, screen->id, 
            attribs, modes[i].width, modes[i].height, modes[i].refreshRate
        );
    }
    return 0;
}

int vioarr_screen_valid(vioarr_screen_t* screen)
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

    vioarr_input_axis_event(1, (int)xoffset, (int)yoffset);    
}

static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    (void)window;
    vioarr_input_scroll_event(1, (int)xoffset, (int)yoffset);
}

/**
 * Modifier conversion methods from GLFW => Asgaard
 */
#define VKC_KEYS_INVALID_4  VKC_INVALID, VKC_INVALID, VKC_INVALID, VKC_INVALID
#define VKC_KEYS_INVALID_8  VKC_KEYS_INVALID_4, VKC_KEYS_INVALID_4
#define VKC_KEYS_INVALID_16 VKC_KEYS_INVALID_8, VKC_KEYS_INVALID_8
#define VKC_KEYS_INVALID_32 VKC_KEYS_INVALID_16, VKC_KEYS_INVALID_16
static uint8_t asgaardKeyCodes[GLFW_KEY_LAST] = {
    // the first 32 keys are invalid
    VKC_KEYS_INVALID_32,

    // Then some non-valid from 32-57
    VKC_SPACE, VKC_APOSTROPHE, VKC_COMMA, VKC_SUBTRACT, VKC_DOT, VKC_SLASH,
    VKC_0, VKC_1, VKC_2, VKC_3, VKC_4, VKC_5, VKC_6, VKC_7, VKC_8, VKC_9,

    // 58, 60 is invalid
    VKC_INVALID,
    VKC_SEMICOLON,
    VKC_INVALID,
    VKC_EQUAL,
    VKC_KEYS_INVALID_4,
    VKC_A, VKC_B, VKC_C, VKC_D, VKC_E, VKC_F, VKC_G, VKC_H, VKC_I, VKC_J, VKC_K, 
    VKC_L, VKC_M, VKC_N, VKC_O, VKC_P, VKC_Q, VKC_R, VKC_S, VKC_T, VKC_U, VKC_V, 
    VKC_W, VKC_X, VKC_Y, VKC_Z,
    VKC_LBRACKET,
    VKC_BACKSLASH,
    VKC_RBRACKET,
    VKC_INVALID, VKC_INVALID, VKC_INVALID,
    VKC_TICK,
    
    // invalid from 97-160, 65 keys
    VKC_KEYS_INVALID_32, VKC_KEYS_INVALID_32, VKC_INVALID,

    // Non-Us codes 2
    VKC_INVALID, VKC_INVALID,

    // 162-256 invalid, 94 keys
    VKC_KEYS_INVALID_32, VKC_KEYS_INVALID_32, VKC_KEYS_INVALID_16, VKC_KEYS_INVALID_8,
    VKC_KEYS_INVALID_4, VKC_INVALID, VKC_INVALID,

    VKC_ESCAPE,
    VKC_ENTER,
    VKC_TAB,
    VKC_BACK,
    VKC_INSERT,
    VKC_DELETE,
    VKC_RIGHT,
    VKC_LEFT,
    VKC_DOWN,
    VKC_UP,
    VKC_PAGEUP,
    VKC_PAGEDOWN,
    VKC_HOME,
    VKC_END,

    // 11 invalids
    VKC_KEYS_INVALID_8, VKC_INVALID, VKC_INVALID, VKC_INVALID,

    VKC_CAPSLOCK,
    VKC_SCROLL,
    VKC_NUMLOCK,
    VKC_PRINT,
    VKC_PAUSE,

    // 6 invalids
    VKC_KEYS_INVALID_4, VKC_INVALID, VKC_INVALID,

    VKC_F1, VKC_F2, VKC_F3, VKC_F4, VKC_F5, VKC_F6, VKC_F7, 
    VKC_F8, VKC_F9, VKC_F10, VKC_F11, VKC_F12, VKC_F13, VKC_F14, 
    VKC_F15, VKC_F16, VKC_F17, VKC_F18, VKC_F19, VKC_F20, 
    VKC_F21, VKC_F22, VKC_F23, VKC_F24,

    // 6 invalids
    VKC_KEYS_INVALID_4, VKC_INVALID, VKC_INVALID,

    // keypad keys
    VKC_0, VKC_1, VKC_2, VKC_3, VKC_4, VKC_5, VKC_6, VKC_7, VKC_8, VKC_9,
    VKC_DECIMAL, VKC_DIVIDE, VKC_MULTIPLY, VKC_SUBTRACT, VKC_ADD, VKC_ENTER,
    VKC_EQUAL,

    // 4 invalid keys
    VKC_KEYS_INVALID_4,

    VKC_LSHIFT,
    VKC_LCONTROL,
    VKC_LALT,
    VKC_INVALID,
    VKC_LWIN,
    VKC_RSHIFT,
    VKC_RCONTROL,
    VKC_RALT,
    VKC_RWIN,
    VKC_APPS
};

static unsigned int convertModifiersToAsgaard(int action, int mods)
{
    unsigned int modifiers = 0;
    if (mods & GLFW_MOD_SHIFT)     modifiers |= VKS_MODIFIER_LSHIFT;
    if (mods & GLFW_MOD_CONTROL)   modifiers |= VKS_MODIFIER_LCTRL;
    if (mods & GLFW_MOD_ALT)       modifiers |= VKS_MODIFIER_LALT;
    if (mods & GLFW_MOD_CAPS_LOCK) modifiers |= VKS_MODIFIER_CAPSLOCK;
    if (mods & GLFW_MOD_NUM_LOCK)  modifiers |= VKS_MODIFIER_NUMLOCK;
    if (action & GLFW_REPEAT)      modifiers |= VKS_MODIFIER_REPEATED;
    return modifiers;
}

static uint32_t asgaardMButtonCodes[GLFW_MOUSE_BUTTON_LAST + 1] = {
    VKC_LBUTTON,
    VKC_RBUTTON,
    VKC_MBUTTON,
    VKC_XBUTTON0,
    VKC_XBUTTON1,
    VKC_XBUTTON2,
    VKC_XBUTTON3,
    VKC_XBUTTON4
};

static void glfw_mouse_key_callback(GLFWwindow* window, int button, int action, int mods)
{
    (void)window;
    vioarr_input_button_event(1, 
        asgaardMButtonCodes[button], 
        convertModifiersToAsgaard(action, mods),
        action == GLFW_RELEASE ? 0 : 1
    );
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)window;
    (void)scancode;
    vioarr_input_button_event(2, 
        asgaardKeyCodes[key], 
        convertModifiersToAsgaard(action, mods), 
        action == GLFW_RELEASE ? 0 : 1
    );
}
