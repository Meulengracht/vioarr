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
 * ValiOS - Application Framework (Asgaard)
 *  - Contains the implementation of the application framework used for building
 *    graphical applications.
 */
 
#include "include/events/key_event.hpp"

namespace {
    // Keymap when modifier SHIFT is present
    static uint8_t g_asciiShiftKeyMap[VKC_KEYCOUNT] = {
        0, ')', '!', '\"', '#', '$', '%', '^', '&', '*', '(', '*',
        '+', '-', '-', ',', '/', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
        'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 
        'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '`', '~', 0, '\t', '>', 
        '<', ':', 0, '\n', 0, 0, '_', 0, ' ', '?', '|', '@', '+',
        '{', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    // Keymap when no modifier is present
    static uint8_t g_asciiKeyMap[VKC_KEYCOUNT] = {
        0, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*',
        '+', '-', '-', ',', '/', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
        'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 
        't', 'u', 'v', 'w', 'x', 'y', 'z', '`', '`', 0, '\t', '.', 
        ',', ';', 0, '\n', 0, 0, '-', 0, ' ', '/', '\\', '\'', '=',
        '[', ']', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    uint32_t TranslateKey(enum VKeyCode keyCode, const uint16_t modifiers)
    {
        bool     shouldUpperCase = modifiers & (VKS_MODIFIER_LSHIFT | VKS_MODIFIER_RSHIFT);
        uint32_t character;

        if (modifiers & VKS_MODIFIER_CAPSLOCK) {
            if (shouldUpperCase != 0) {
                shouldUpperCase = 0;
            }
            else {
                shouldUpperCase = 1;
            }
        }

        // Handle modifiers, caps lock negates shift as seen above
        if (shouldUpperCase) {
            character = g_asciiShiftKeyMap[keyCode];
        }
        else {
            character = g_asciiKeyMap[keyCode];
        }
        return character;
    }
}

namespace Asgaard {
    KeyEvent::KeyEvent(const uint32_t keyCode, const uint16_t modifiers, bool pressed)
        : Event(Event::Type::KEY_EVENT)
        , m_key(TranslateKey(static_cast<enum VKeyCode>(keyCode), modifiers))
        , m_modifiers(modifiers)
        , m_pressed(pressed)
        , m_keyCode(static_cast<enum VKeyCode>(keyCode))
    {
        
    }
    
    uint32_t KeyEvent::Key() const
    {
        return m_key;
    }
    
    enum VKeyCode KeyEvent::KeyCode() const
    {
        return m_keyCode;
    }
    
    bool KeyEvent::Pressed() const
    {
        return m_pressed;
    }

    bool KeyEvent::IsRepeat() const
    {
        return (m_modifiers & VKS_MODIFIER_REPEATED) == VKS_MODIFIER_REPEATED;
    }

    bool KeyEvent::LeftControl() const
    {
        return (m_modifiers & VKS_MODIFIER_LCTRL) == 0;
    }

    bool KeyEvent::RightControl() const
    {
        return (m_modifiers & VKS_MODIFIER_RCTRL) == 0;
    }

    bool KeyEvent::Control() const
    {
        return LeftControl() || RightControl();
    }
}
