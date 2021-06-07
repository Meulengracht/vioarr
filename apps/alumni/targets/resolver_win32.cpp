/* MollenOS
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
 * MollenOS Terminal Implementation (Alumnious)
 * - The terminal emulator implementation for Vali. Built on manual rendering and
 *   using freetype as the font renderer.
 */

#include <chrono>
#include <thread>
#include <sstream>
#include "../terminal_interpreter.hpp"
#include "../terminal.hpp"
#include "resolver_win32.hpp"
#include <windows.h>
#include "dirent.h"

namespace {
    int Vk2Ascii(unsigned int KeyCode, unsigned int ScanCode, int *Character)
    {
        unsigned char KybdState[256];

        if (!GetKeyboardState(&KybdState[0])) {
            return 0;
        }
        return (ToAscii(KeyCode, ScanCode, &KybdState[0], (LPWORD)Character, 0) > 0);
    }
}

CWin32Alumni::CWin32Alumni(std::unique_ptr<CTerminal> Terminal, std::unique_ptr<CTerminalInterpreter> Interpreter)
    : CAlumni(std::move(Terminal), std::move(Interpreter))
{
}

bool CWin32Alumni::HandleKeyCode(unsigned int KeyCode, unsigned int Flags)
{
    return true;
}

void CWin32Alumni::PrintCommandHeader()
{
    // Dont print the command header if an application is running
    if (m_application <= 0) {
        m_terminal->Print("\033[34m%s@%s~\033[39m ", m_profile.c_str(), m_currentDirectory.c_str());
    }
}

bool CWin32Alumni::CommandResolver(const std::string&, const std::vector<std::string>&)
{
    return false;
}

bool CWin32Alumni::ListDirectory(const std::vector<std::string>&)
{
    return false;
}

bool CWin32Alumni::ChangeDirectory(const std::vector<std::string>&)
{
    return false;
}
