/* ValiOS
 *
 * Copyright 2018, Philip Meulengracht
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
#pragma once

#include "config.hpp"

struct wm_keyboard_key_event;

namespace Asgaard {
    class ASGAARD_API KeyEvent {
    public:
        KeyEvent(struct wm_keyboard_key_event*);
        
        char          KeyAscii() const;
        unsigned int  KeyUnicode() const;
        unsigned char KeyCode() const;
        bool          Pressed() const;
        bool          IsRepeat() const;

        bool LeftControl() const;
        bool RightControl() const;
        bool Control() const;
        
    private:
        unsigned int  m_modifiers;
        unsigned int  m_keyUnicode;
        char          m_keyAscii;
        unsigned char m_keyCode;
    };
}
