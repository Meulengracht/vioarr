/**
 * Copyright 2021, Philip Meulengracht
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
 * Application Framework (Vioarr)
 *  - Contains the implementation of the application framework used for building
 *    graphical applications.
 */

#ifndef __COMMON_VKEYCODES_H__
#define __COMMON_VKEYCODES_H__

enum VKeyCode {
    VKC_INVALID              = 0x00,

    // 0-9
    VKC_0                    = 0x01,
    VKC_1                    = 0x02,
    VKC_2                    = 0x03,
    VKC_3                    = 0x04,
    VKC_4                    = 0x05,
    VKC_5                    = 0x06,
    VKC_6                    = 0x07,
    VKC_7                    = 0x08,
    VKC_8                    = 0x09,
    VKC_9                    = 0x0A,
    VKC_MULTIPLY             = 0x0B,
    VKC_ADD                  = 0x0C,
    VKC_SEPARATOR            = 0x0D,
    VKC_SUBTRACT             = 0x0E,
    VKC_DECIMAL              = 0x0F,
    VKC_DIVIDE               = 0x10,

    // A-Z
    VKC_A                    = 0x11,
    VKC_B                    = 0x12,
    VKC_C                    = 0x13,
    VKC_D                    = 0x14,
    VKC_E                    = 0x15,
    VKC_F                    = 0x16,
    VKC_G                    = 0x17,
    VKC_H                    = 0x18,
    VKC_I                    = 0x19,
    VKC_J                    = 0x1A,
    VKC_K                    = 0x1B,
    VKC_L                    = 0x1C,
    VKC_M                    = 0x1D,
    VKC_N                    = 0x1E,
    VKC_O                    = 0x1F,
    VKC_P                    = 0x20,
    VKC_Q                    = 0x21,
    VKC_R                    = 0x22,
    VKC_S                    = 0x23,
    VKC_T                    = 0x24,
    VKC_U                    = 0x25,
    VKC_V                    = 0x26,
    VKC_W                    = 0x27,
    VKC_X                    = 0x28,
    VKC_Y                    = 0x29,
    VKC_Z                    = 0x2A,

    VKC_TICK                 = 0x2B, // `
    VKC_BACKTICK             = 0x2C, // ´
    VKC_BACK                 = 0x2D, // backspace
    VKC_TAB                  = 0x2E,
    VKC_DOT                  = 0x2F,
    VKC_COMMA                = 0x30,
    VKC_SEMICOLON            = 0x31,
    VKC_CLEAR                = 0x32,
    VKC_ENTER                = 0x33,
    VKC_PAUSE                = 0x34,
    VKC_CAPSLOCK             = 0x35,
    VKC_HYPHEN               = 0x36,
    VKC_ESCAPE               = 0x37,
    VKC_SPACE                = 0x38,
    VKC_SLASH                = 0x39,
    VKC_BACKSLASH            = 0x3A,
    VKC_APOSTROPHE           = 0x3B,
    VKC_EQUAL                = 0x3C,
    VKC_LBRACKET             = 0x3D,
    VKC_RBRACKET             = 0x3E,
    VKC_PAGEUP               = 0x3F,
    VKC_PAGEDOWN             = 0x40,
    VKC_END                  = 0x41,
    VKC_HOME                 = 0x42,
    VKC_LEFT                 = 0x43,
    VKC_UP                   = 0x44,
    VKC_RIGHT                = 0x45,
    VKC_DOWN                 = 0x46,
    VKC_SELECT               = 0x47,
    VKC_PRINT                = 0x48,
    VKC_EXECUTE              = 0x49,
    VKC_SNAPSHOT             = 0x4A,
    VKC_INSERT               = 0x4B,
    VKC_DELETE               = 0x4C,
    VKC_HELP                 = 0x4D,

    VKC_NUMLOCK              = 0x4E,
    VKC_SCROLL               = 0x4F,

    VKC_LSHIFT               = 0x50,
    VKC_RSHIFT               = 0x51,
    VKC_LCONTROL             = 0x52,
    VKC_RCONTROL             = 0x53,
    VKC_LALT                 = 0x54,
    VKC_RALT                 = 0x55,

    VKC_LWIN                 = 0x56,
    VKC_RWIN                 = 0x57,
    VKC_APPS                 = 0x58,

    VKC_F1                   = 0x59,
    VKC_F2                   = 0x5A,
    VKC_F3                   = 0x5B,
    VKC_F4                   = 0x5C,
    VKC_F5                   = 0x5D,
    VKC_F6                   = 0x5E,
    VKC_F7                   = 0x5F,
    VKC_F8                   = 0x60,
    VKC_F9                   = 0x61,
    VKC_F10                  = 0x62,
    VKC_F11                  = 0x63,
    VKC_F12                  = 0x64,
    VKC_F13                  = 0x65,
    VKC_F14                  = 0x66,
    VKC_F15                  = 0x67,
    VKC_F16                  = 0x68,
    VKC_F17                  = 0x69,
    VKC_F18                  = 0x6A,
    VKC_F19                  = 0x6B,
    VKC_F20                  = 0x6C,
    VKC_F21                  = 0x6D,
    VKC_F22                  = 0x6E,
    VKC_F23                  = 0x6F,
    VKC_F24                  = 0x70,

    VKC_BROWSER_BACK         = 0x71,
    VKC_BROWSER_FORWARD      = 0x72,
    VKC_BROWSER_REFRESH      = 0x73,
    VKC_BROWSER_STOP         = 0x74,
    VKC_BROWSER_SEARCH       = 0x75,
    VKC_BROWSER_FAVORITES    = 0x76,
    VKC_BROWSER_HOME         = 0x77,
    VKC_VOLUME_MUTE          = 0x78,
    VKC_VOLUME_DOWN          = 0x79,
    VKC_VOLUME_UP            = 0x7A,
    VKC_MEDIA_NEXT_TRACK     = 0x7B,
    VKC_MEDIA_PREV_TRACK     = 0x7C,
    VKC_MEDIA_STOP           = 0x7D,
    VKC_MEDIA_PLAY_PAUSE     = 0x7E,
    VKC_LAUNCH_MAIL          = 0x7F,
    VKC_LAUNCH_MEDIA_SELECT  = 0x80,
    VKC_LAUNCH_APP1          = 0x81,
    VKC_LAUNCH_APP2          = 0x82,
    VKC_POWER                = 0x83,
    VKC_WAKE                 = 0x84,
    VKC_SLEEP                = 0x85,
    VKC_PLAY                 = 0x86,
    VKC_ZOOM                 = 0x87,

    VKC_LBUTTON              = 0x88,
    VKC_MBUTTON              = 0x89,
    VKC_RBUTTON              = 0x8A,
    VKC_XBUTTON0             = 0x8B,
    VKC_XBUTTON1             = 0x8C,
    VKC_XBUTTON2             = 0x8D,
    VKC_XBUTTON3             = 0x8E,
    VKC_XBUTTON4             = 0x8F,
    VKC_XBUTTON5             = 0x90,
    VKC_XBUTTON6             = 0x91,
    VKC_XBUTTON7             = 0x92,

    VKC_KEYCOUNT             = 0x93
};

enum VKeyState {
    VKS_MODIFIER_LSHIFT    = 0x1,
    VKS_MODIFIER_RSHIFT    = 0x2,
    VKS_MODIFIER_LALT      = 0x4,
    VKS_MODIFIER_RALT      = 0x8,
    VKS_MODIFIER_LCTRL     = 0x10,
    VKS_MODIFIER_RCTRL     = 0x20,
    VKS_MODIFIER_SCROLLOCK = 0x40,
    VKS_MODIFIER_NUMLOCK   = 0x80,
    VKS_MODIFIER_CAPSLOCK  = 0x100,
    VKS_MODIFIER_REPEATED  = 0x200
};

#endif //! __COMMON_VKEYCODES_H__
