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

#include <functional>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <memory>
#include <list>

class TerminalInterpreter {
    class TerminalCommand {
    public:
        TerminalCommand(const std::string& Command, const std::string& Description, std::function<bool(const std::vector<std::string>&)> Fn)
            : m_Command(Command), m_Description(Description), m_Function(Fn) { }

        int operator()(const std::string& Command, const std::vector<std::string>& Arguments) {
            int Result = GetCommandDistance(Command);
            if (!Result) {
                m_Function(Arguments);
            }
            return Result;
        }

        const std::string& GetCommandText() const { return m_Command; }
        const std::string& GetDescription() const { return m_Description; }

    private:
        int GetCommandDistance(const std::string& Command);

    private:
        std::string m_Command;
        std::string m_Description;
        
        std::function<bool(const std::vector<std::string>&)> m_Function;
    };

public:
    TerminalInterpreter();
    ~TerminalInterpreter() = default;

    bool Interpret(const std::string& String);
    void RegisterCommand(const std::string& Command, const std::string& Description, std::function<bool(const std::vector<std::string>&)> Fn);
    
    const std::string&  GetClosestMatch() const { return m_ClosestMatch; }
    const std::list<std::unique_ptr<TerminalCommand>>& GetCommands() const { return m_Commands; }

protected:
    virtual bool CommandResolver(const std::string&, const std::vector<std::string>&) = 0;
    std::vector<std::string> SplitCommandString(const std::string&, std::vector<char> seperators = { ' ' });

private:
    std::list<std::unique_ptr<TerminalCommand>> m_Commands;
    std::string                                 m_ClosestMatch;
};
