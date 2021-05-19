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
 
#include "include/events/key_event.hpp"

namespace {
    uint32_t TranslateKey(enum VKeyCode keyCode, const uint16_t modifiers)
    {

        return 0;
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
