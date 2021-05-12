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

#include "terminal.hpp"
#include <map>

static std::map<int, Asgaard::Drawing::Color*> g_colorMap;

void Terminal::InitializeVT()
{
    g_colorMap.insert(std::make_pair(0, new Asgaard::Drawing::Color(0xFF, 0, 0, 0)));          // black
    g_colorMap.insert(std::make_pair(1, new Asgaard::Drawing::Color(0xFF, 0xFF, 0, 0)));       // red
    g_colorMap.insert(std::make_pair(2, new Asgaard::Drawing::Color(0xFF, 0, 0xFF, 0)));       // green
    g_colorMap.insert(std::make_pair(3, new Asgaard::Drawing::Color(0xFF, 0xFE, 0xFE, 0x33))); // yellow
    g_colorMap.insert(std::make_pair(4, new Asgaard::Drawing::Color(0xFF, 0x0C, 0x8F, 0xEE))); // blue
    g_colorMap.insert(std::make_pair(5, new Asgaard::Drawing::Color(0xFF, 0xFF, 0x00, 0xFF))); // magenta
    g_colorMap.insert(std::make_pair(6, new Asgaard::Drawing::Color(0xFF, 0x00, 0xFF, 0xFF))); // cyan
    g_colorMap.insert(std::make_pair(7, new Asgaard::Drawing::Color(0xFF, 0xFF, 0xFF, 0xFF))); // white
    g_colorMap.insert(std::make_pair(8, new Asgaard::Drawing::Color(0xFF, 0xFF, 0xFF, 0xFF))); // invalid, white
}

size_t Terminal::HandleVTEscapeCode(const char* buffer)
{
    char*  endOfCode;
    long   code = std::strtol(buffer, &endOfCode, 10);

    // Text attributes 0-8
    if (code >= 0 && code < 9) {
        // 0 - reset
        // 1 - bold
        // 2 - dim
        // 3 - standout
        // 4 - underscore
        // 5 - blink
        // 7 - reverse
        // 8 - hidden
    }

    // Text coloring 30-39
    if (code >= 30 && code < 40) {
        int index = (int)code - 30;
        if (code < 39) {
            auto color = g_colorMap[index];
            m_textState.m_fgColor = *color;
        }
        else {
            m_textState.m_fgColor = m_textState.m_defaultFgColor;
        }
    }

    // Background coloring
    if (code >= 40 && code < 50) {
        int index = (int)code - 30;
        if (code < 39) {
            auto color = g_colorMap[index];
            m_textState.m_bgColor = *color;
        }
        else {
            m_textState.m_bgColor = m_textState.m_defaultBgColor;
        }
    }

    return (size_t)(endOfCode - buffer);
}

size_t Terminal::ParseVTEscapeCode(const char* buffer)
{
    size_t itr = 0;
    if (buffer[itr] == '\033') {
        itr++;
    }

    // validate and skip over [
    if (buffer[itr] != '[') {
        return 0;
    }
    itr++;

    while (buffer[itr] && buffer[itr] != 'm') {
        if (buffer[itr] == ';') {
            itr++;
        }
        itr += HandleVTEscapeCode(&buffer[itr]);
    }
    
    // skip over 'm'
    itr++;
    return itr;
}
