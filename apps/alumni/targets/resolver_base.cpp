/**
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

#include "../terminal_interpreter.hpp"
#include "../terminal.hpp"
#include "resolver_base.hpp"

ResolverBase::ResolverBase()
    : m_terminal(nullptr)
{
    // Register inbuilt commands
    RegisterCommand("cd", "Change the working directory", std::bind(&ResolverBase::ChangeDirectory, this, std::placeholders::_1));
    RegisterCommand("ls", "Lists the contents of the current working directory", std::bind(&ResolverBase::ListDirectory, this, std::placeholders::_1));
    RegisterCommand("help", "Lists all registered terminal commands", std::bind(&ResolverBase::Help, this, std::placeholders::_1));
    RegisterCommand("exit", "Quits the terminal", std::bind(&ResolverBase::Exit, this, std::placeholders::_1));
}

void ResolverBase::SetTerminal(std::shared_ptr<Terminal>& terminal)
{
    m_terminal = terminal;
}

bool ResolverBase::Help(const std::vector<std::string>&)
{
    if (m_terminal == nullptr) {
        return false;
    }

    auto longestCommand = 0U;
    for (auto& command : GetCommands()) {
        if (command->GetCommandText().size() > longestCommand) {
            longestCommand = command->GetCommandText().size();
        }
    }

    for (auto& command : GetCommands()) {
        m_terminal->Print("%-*s %s\n", longestCommand, command->GetCommandText().c_str(), command->GetDescription().c_str());
    }
    return true;
}

bool ResolverBase::Exit(const std::vector<std::string>&)
{
    exit(EXIT_SUCCESS);
    return true;
}

void ResolverBase::DirectoryPrinter(const std::vector<std::string>& directoryEntries)
{
    auto longestEntry = 0U;
    auto entireLength = 0U;
    for (const auto& entry : directoryEntries) {
        if (entry.size() > longestEntry) {
            longestEntry = entry.size();
        }
        entireLength += entry.size();
    }

    // account for spaces
    entireLength += std::min(1UL, directoryEntries.size()) - 1;

    auto printEndOfLine = [&] (bool condition) {
        if (condition) {
            m_terminal->Print("\n");
        }
        else {
            m_terminal->Print(" ");
        }
    };

    // account for a space after each entry
    if (m_terminal->GetNumberOfCellsPerLine() >= static_cast<int>(entireLength)) {
        for (auto i = 0U; i < directoryEntries.size(); i++) {
            m_terminal->Print("%s", directoryEntries[i].c_str());
            printEndOfLine(i == directoryEntries.size() - 1);
        }
    }
    else {
        auto entriesPerLine = m_terminal->GetNumberOfCellsPerLine() / (longestEntry + 1);
        for (auto i = 0U; i < directoryEntries.size(); i++) {
            m_terminal->Print("%-*s", longestEntry, directoryEntries[i].c_str());
            printEndOfLine(
                (((i + 1) % entriesPerLine) == 0) || 
                (i == (directoryEntries.size() - 1))
            );
        }
    }
}
