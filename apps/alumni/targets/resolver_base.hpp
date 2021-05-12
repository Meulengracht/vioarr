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
#pragma once

#include <asgaard/surface.hpp>
#include <memory>
#include <string>
#include "../terminal_interpreter.hpp"
#include <thread>
#include <vector>

class Terminal;

class ResolverBase : public TerminalInterpreter {
public:
    ResolverBase();
    virtual ~ResolverBase() = default;

    void SetTerminal(std::shared_ptr<Terminal>&);

public:
    virtual bool HandleKeyCode(const Asgaard::KeyEvent&) = 0;
    virtual void PrintCommandHeader() = 0;

protected:
    virtual bool ListDirectory(const std::vector<std::string>&) = 0;
    virtual bool ChangeDirectory(const std::vector<std::string>&) = 0;
    bool Help(const std::vector<std::string>&);
    bool Exit(const std::vector<std::string>&);

protected:
    std::shared_ptr<Terminal> m_terminal;
};
