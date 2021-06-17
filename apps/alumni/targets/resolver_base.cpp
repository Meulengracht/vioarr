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

#include <events/key_event.hpp>
#include "../terminal_interpreter.hpp"
#include "../terminal.hpp"
#include "resolver_base.hpp"

ResolverBase::ResolverBase()
    : m_terminal(nullptr)
    , m_currentDirectory("nil")
    , m_autoCompleteIndex(0)
    , m_lastKeyCode(VKC_INVALID)
    , m_originalCommand("")
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

bool ResolverBase::ListDirectory(const std::vector<std::string>& arguments)
{
    auto path = m_currentDirectory;
    if (arguments.size() != 0) {
        path = arguments[0];
    }

    auto directoryEntries = GetDirectoryContents(path);
    DirectoryPrinter(directoryEntries);
    return true;
}

void ResolverBase::DirectoryPrinter(const std::vector<DirectoryEntry>& directoryEntries)
{
    auto longestEntry = 0U;
    auto entireLength = 0U;
    for (const auto& entry : directoryEntries) {
        if (entry.GetName().size() > longestEntry) {
            longestEntry = entry.GetName().size();
        }
        entireLength += entry.GetName().size();
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

    auto getFormatString = [] (const auto& entry, bool padding) {
        if (entry.GetType() == DirectoryEntry::Type::DIRECTORY) {
            return padding ? "\033[34m%-*s" : "\033[34m%s";
        }
        else if (entry.GetType() == DirectoryEntry::Type::EXECUTABLE) {
            return padding ? "\033[32m%-*s" : "\033[32m%s";
        }
        return padding ? "\033[39m%-*s" : "\033[39m%s";
    };

    // account for a space after each entry
    if (m_terminal->GetNumberOfCellsPerLine() >= static_cast<int>(entireLength)) {
        for (auto i = 0U; i < directoryEntries.size(); i++) {
            m_terminal->Print(
                getFormatString(directoryEntries[i], false), 
                directoryEntries[i].GetName().c_str()
            );
            printEndOfLine(i == directoryEntries.size() - 1);
        }
    }
    else {
        auto entriesPerLine = m_terminal->GetNumberOfCellsPerLine() / (longestEntry + 1);
        for (auto i = 0U; i < directoryEntries.size(); i++) {
            m_terminal->Print(
                getFormatString(directoryEntries[i], true),
                longestEntry,
                directoryEntries[i].GetName().c_str()
            );
            printEndOfLine(
                (((i + 1) % entriesPerLine) == 0) || 
                (i == (directoryEntries.size() - 1))
            );
        }
    }
}

bool ResolverBase::HandleKeyCode(const Asgaard::KeyEvent& key)
{
    if (key.KeyCode() == VKC_TAB && key.Pressed()) {
        if (m_lastKeyCode != VKC_TAB) {
            m_originalCommand = m_terminal->GetCurrentInput();
            m_autoCompleteIndex = 0;
        }
        else {
            m_autoCompleteIndex++;
        }
        TryAutoComplete();
        return true;
    }
    m_lastKeyCode = key.KeyCode();
    return false;
}

void ResolverBase::TryAutoComplete()
{
    if (!m_originalCommand.size()) {
        return;
    }

    // if we double tab then list all files, and restore the input line
    auto directoryEntries = GetDirectoryContents(m_currentDirectory);
    if (m_autoCompleteIndex > 0) {
        m_terminal->ClearInput(false);
        DirectoryPrinter(directoryEntries);
        PrintCommandHeader();
        m_terminal->SetInput(m_originalCommand);
        m_terminal->RequestRedraw();
        return;
    }

    // otherwise we try to complete the line
    auto i = 0;
    auto lastToken = SplitCommandString(m_originalCommand).back();
    auto matchingEntry = std::find_if(
        std::begin(directoryEntries), 
        std::end(directoryEntries),
        [targetIndex = m_autoCompleteIndex, lastToken, &i](const DirectoryEntry& entry) {
            if (entry.GetName().rfind(lastToken, 0) == 0) {
                if (i == targetIndex) {
                    return true;
                }
                i++;
            }
            return false;
        }
    );
    if (matchingEntry != std::end(directoryEntries)) {
        // extend current command with the matching entry
        auto lastMatch  = m_originalCommand.find_last_of(lastToken);
        auto cutCommand = m_originalCommand.substr(0, lastMatch);
        cutCommand += (*matchingEntry).GetName();

        m_terminal->SetInput(cutCommand);
        m_terminal->RequestRedraw();
    }
}
