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

#include <sstream>
#include <numeric>
#include "terminal_interpreter.hpp"

int TerminalInterpreter::TerminalCommand::GetCommandDistance(const std::string& Command)
{
	int s1len = m_Command.size();
	int s2len = Command.size();
	
	auto column_start   = (decltype(s1len))1;
	auto column         = new decltype(s1len)[s1len + 1];
	std::iota(column + column_start - 1, column + s1len + 1, column_start - 1);
	
	for (auto x = column_start; x <= s2len; x++) {
		column[0] = x;
		auto last_diagonal = x - column_start;
		for (auto y = column_start; y <= s1len; y++) {
			auto old_diagonal = column[y];
			auto possibilities = {
				column[y] + 1,
				column[y - 1] + 1,
				last_diagonal + (m_Command[y - 1] == Command[x - 1]? 0 : 1)
			};
			column[y] = std::min(possibilities);
			last_diagonal = old_diagonal;
		}
	}
	auto result = column[s1len];
	delete[] column;
	return result;
}

TerminalInterpreter::TerminalInterpreter()
    : m_ClosestMatch("")
{
}

void TerminalInterpreter::RegisterCommand(const std::string& Command, const std::string& Description, std::function<bool(const std::vector<std::string>&)> Fn)
{
    m_Commands.push_back(std::make_unique<TerminalCommand>(Command, Description, Fn));
}

std::vector<std::string> TerminalInterpreter::SplitCommandString(const std::string& command)
{
    std::vector<std::string> tokens;
    std::string              token = "";

    for (auto i = 0u; i < command.size(); i++) {
        if (command[i] == ' ' || command[i] == '/' || command[i] == '\\') {
            if (token != "") {
                tokens.push_back(token);
            }
            token = "";
            continue;
        }
        token.push_back(command[i]);
    }
    if (token != "") {
        tokens.push_back(token);
    }
    return tokens;
}

bool TerminalInterpreter::Interpret(const std::string& String)
{
    int shortestDistance = String.size();
    m_ClosestMatch = "";
    if (String.size() == 0) {
        return false;
    }
    
    // Get command and remove from array
    auto tokens        = SplitCommandString(String);
    auto commandString = tokens[0];
    tokens.erase(tokens.begin());

    for (auto& command : m_Commands) {
        int distance = command->operator()(commandString, tokens);
        if (distance == 0) {
            return true;
        }
        else if (distance < shortestDistance) {
            shortestDistance    = distance;
            m_ClosestMatch      = command->GetCommandText();
        }
    }
    return CommandResolver(commandString, tokens);
}
