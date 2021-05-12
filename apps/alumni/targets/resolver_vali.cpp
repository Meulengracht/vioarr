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

#include <asgaard/application.hpp>
#include <asgaard/key_event.hpp>
#include <os/mollenos.h>
#include <os/process.h>
#include <os/keycodes.h>
#include <io.h>
#include <ioset.h>
#include <event.h>
#include "../terminal_interpreter.hpp"
#include "../terminal.hpp"
#include "resolver_vali.hpp"

#define __TRACE
#include <ddk/utils.h>


namespace {
    static bool EndsWith(const std::string& String, const std::string& Suffix)
    {
        return String.size() >= Suffix.size() && 0 == String.compare(String.size() - Suffix.size(), Suffix.size(), Suffix);
    }
}

ResolverVali::ResolverVali(int stdoutDescriptor, int stderrDescriptor)
    : ResolverBase()
    , m_profile("philip")
    , m_currentDirectory("n/a")
    , m_application(UUID_INVALID)
    , m_stdoutDescriptor(stdoutDescriptor)
    , m_stdinDescriptor(pipe(0x1000, 0))
    , m_stderrDescriptor(stderrDescriptor)
    , m_processEvent(-1)
{
    UpdateWorkingDirectory();
}

ResolverVali::~ResolverVali()
{
    close(m_stdinDescriptor);
}

void ResolverVali::Setup(const std::shared_ptr<ResolverVali>& selfReference)
{
    m_processEvent = eventd(0, EVT_RESET_EVENT);
    if (m_processEvent < 0) {
        // throw
    }

    Asgaard::APP.AddEventDescriptor(m_processEvent, IOSETSYN, selfReference);
}

void ResolverVali::UpdateWorkingDirectory()
{
    char* CurrentPath = (char*)std::malloc(_MAXPATH);
    std::memset(CurrentPath, 0, _MAXPATH);
    if (GetWorkingDirectory(CurrentPath, _MAXPATH) == OsSuccess) {
        m_currentDirectory = std::string(CurrentPath);
    }
    else {
        m_currentDirectory = "n/a";
    }
    std::free(CurrentPath);
}

bool ResolverVali::HandleKeyCode(const Asgaard::KeyEvent& key)
{
    if (m_application != UUID_INVALID) {
        if (key.KeyCode() == VK_C && !key.Pressed() && key.LeftControl()) {
            // send signal to terminate
            ProcessKill(m_application);
        }
        else {
            // redirect to running application
            // @todo support unicode
            char data = key.KeyAscii();
            write(m_stdinDescriptor, &data, sizeof(data));
        }
        return true;
    }
    return false;
}

void ResolverVali::PrintCommandHeader()
{
    // Dont print the command header if an application is running
    if (m_application == UUID_INVALID) {
        m_terminal->Print("\033[34m%s@%s~\033[39m ", m_profile.c_str(), m_currentDirectory.c_str());
    }
}

void ResolverVali::DescriptorEvent(int iod, unsigned int events)
{
    if (iod == m_processEvent) {
        int exitCode;
        
        read(iod, &exitCode, sizeof(int));
        m_terminal->Print("process exitted with code %i\n", exitCode);
        m_application = UUID_INVALID;
        PrintCommandHeader();
    }
}

void ResolverVali::WaitForProcess()
{
    int exitCode = 0;

    ProcessJoin(m_application, 0, &exitCode);
    write(m_processEvent, &exitCode, sizeof(int));
}

bool ResolverVali::ExecuteProgram(const std::string& Program, const std::vector<std::string>& Arguments)
{
    ProcessConfiguration_t configuration;
    std::string            line = "";

    // Set arguments
    if (Arguments.size() != 0) {
        for (int i = 0; i < Arguments.size(); i++) {
            if (i != 0) {
                line += " ";
            }
            line += Arguments[i];
        }
    }
    
    ProcessConfigurationInitialize(&configuration);

    // Set inheritation
    configuration.InheritFlags = PROCESS_INHERIT_STDOUT | PROCESS_INHERIT_STDIN | PROCESS_INHERIT_STDERR;
    configuration.StdOutHandle = m_stdoutDescriptor;
    configuration.StdInHandle  = m_stdinDescriptor;
    configuration.StdErrHandle = m_stderrDescriptor;
    
    ProcessSpawnEx(Program.c_str(), line.c_str(), &configuration, &m_application);
    if (m_application != UUID_INVALID) {
        std::thread spawn(std::bind(&ResolverVali::WaitForProcess, this));
        spawn.detach();
        return true;
    }
    return false;
}

std::vector<std::string> ResolverVali::GetDirectoryContents(const std::string& Path)
{
    struct DIR*              dir;
    struct DIRENT            dp;
    std::vector<std::string> entries;
    TRACE("ResolverVali::GetDirectoryContents(path=%s)", Path.c_str());

    if (opendir(Path.c_str(), 0, &dir) != -1) {
        while (readdir(dir, &dp) != -1) {
            TRACE("ResolverVali::GetDirectoryContents entry %s", &dp.d_name[0]);
            entries.push_back(std::string(&dp.d_name[0]));
        }
        closedir(dir);
    }

    std::sort(entries.begin(), entries.end());
    return entries;
}

bool ResolverVali::IsProgramPathValid(const std::string& Path)
{
    OsFileDescriptor_t Stats = { 0 };

    if (GetFileInformationFromPath(Path.c_str(), &Stats) == OsSuccess) {
        if (!(Stats.Flags & FILE_FLAG_DIRECTORY) && (Stats.Permissions & FILE_PERMISSION_EXECUTE)) {
            return true;
        }
        m_terminal->Print("%s: not an executable file 0x%x\n", Path.c_str(), Stats.Permissions);
    }
    return false;
}

bool ResolverVali::CommandResolver(const std::string& Command, const std::vector<std::string>& Arguments)
{
    std::string ProgramName = Command;
    std::string ProgramPath = "";

    if (!EndsWith(ProgramName, ".app")) {
        ProgramName += ".app";
    }

    // Guess the path of requested application, right now only working
    // directory and $bin is supported. Should we support apps that don't have .app? ...
    ProgramPath = "$bin/" + ProgramName;
    if (!IsProgramPathValid(ProgramPath)) {
        char TempBuffer[_MAXPATH] = { 0 };
        if (GetWorkingDirectory(&TempBuffer[0], _MAXPATH) == OsSuccess) {
            std::string CwdPath(&TempBuffer[0]);
            ProgramPath = CwdPath + "/" + ProgramName;
            if (!IsProgramPathValid(ProgramPath)) {
                m_terminal->Print("%s: not a valid command\n", Command.c_str());
                return false;
            }
        }
        else {
            return false;
        }
    }
    return ExecuteProgram(ProgramPath, Arguments);
}

bool ResolverVali::ListDirectory(const std::vector<std::string>& Arguments)
{
    TRACE("ResolverVali::ListDirectory()");
    auto path = m_currentDirectory;
    if (Arguments.size() != 0) {
        path = Arguments[0];
    }

    auto directoryEntries = GetDirectoryContents(path);
    int  longestEntry = 0;
    for (const auto& entry : directoryEntries) {
        if (entry.size() > longestEntry) {
            longestEntry = entry.size();
        }
    }

    // account for a space after each entry
    int entriesPerLine = m_terminal->GetNumberOfCellsPerLine() / (longestEntry + 1);
    TRACE("ResolverVali::ListDirectory entriesPerLine=%i, longestEntry=%i", entriesPerLine, longestEntry);
    for (int i = 0; i < directoryEntries.size(); i++) {
        // if all entries fit on one line, we don't pad
        if (entriesPerLine >= directoryEntries.size()) {
            m_terminal->Print("%s", directoryEntries[i].c_str());
        }
        else {
            m_terminal->Print("%-*s", longestEntry, directoryEntries[i].c_str());
        }

        // if we reach the max entries per line or we are the last entry, we newline instead of space
        if ((i != 0 && (i % entriesPerLine) == 0) || i == (directoryEntries.size() - 1)) {
            m_terminal->Print("\n");
        }
        else {
            m_terminal->Print(" ");
        }
    }
    return true;
}

bool ResolverVali::ChangeDirectory(const std::vector<std::string>& Arguments)
{
    if (Arguments.size() != 0) {
        std::string Path = Arguments[0];
        if (SetWorkingDirectory(Path.c_str()) == OsSuccess) {
            UpdateWorkingDirectory();
            return true;
        }
        m_terminal->Print("cd: invalid argument %s\n", Path.c_str());
        return false;
    }
    m_terminal->Print("cd: no argument given\n");
    return false;
}
