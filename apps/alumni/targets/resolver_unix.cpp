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

#include <application.hpp>
#include <events/key_event.hpp>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include "../terminal_interpreter.hpp"
#include "../terminal.hpp"
#include "resolver_unix.hpp"
#include <linux/limits.h>
#include <cstring>

namespace {
    static bool EndsWith(const std::string& String, const std::string& Suffix)
    {
        return String.size() >= Suffix.size() && 0 == String.compare(String.size() - Suffix.size(), Suffix.size(), Suffix);
    }
}

ResolverUnix::ResolverUnix(const int* stdoutFds, const int* stderrFds, const int* stdinFds)
    : ResolverBase()
    , m_profile("philip")
    , m_currentDirectory("n/a")
    , m_application(0)
    , m_stdoutFds{stdoutFds[0], stdoutFds[1]}
    , m_stdinFds{stdinFds[0], stdinFds[1]}
    , m_stderrFds{stderrFds[0], stderrFds[1]}
    , m_processEvent(-1)
{
    UpdateWorkingDirectory();

    m_processEvent = eventfd(0, 0);
    if (m_processEvent < 0) {
        // throw
    }

    Asgaard::APP.AddEventDescriptor(m_processEvent, EPOLLIN, shared_from_this());
}

ResolverUnix::~ResolverUnix()
{
    close(m_processEvent);
}

void ResolverUnix::UpdateWorkingDirectory()
{
    char* CurrentPath = (char*)std::malloc(PATH_MAX);
    std::memset(CurrentPath, 0, PATH_MAX);
    if (getcwd(CurrentPath, PATH_MAX - 1) != nullptr) {
        m_currentDirectory = std::string(CurrentPath);
    }
    else {
        m_currentDirectory = "n/a";
    }
    std::free(CurrentPath);
}

bool ResolverUnix::HandleKeyCode(const Asgaard::KeyEvent& key)
{
    if (m_application >= 0) {
        if (key.KeyCode() == VKC_C && !key.Pressed() && key.LeftControl()) {
            // send signal to terminate
            kill(m_application, SIGKILL);
        }
        else {
            // redirect to running application
            auto data = key.Key();
            write(m_stdinFds[PIPE_WRITE], &data, sizeof(uint8_t));
        }
        return true;
    }
    return false;
}

void ResolverUnix::PrintCommandHeader()
{
    // Dont print the command header if an application is running
    if (m_application == 0) {
        m_terminal->Print("\033[34m%s@%s~\033[39m ", m_profile.c_str(), m_currentDirectory.c_str());
    }
}

void ResolverUnix::DescriptorEvent(int iod, unsigned int events)
{
    if (iod == m_processEvent) {
        uint64_t exitCode;
        
        read(iod, &exitCode, sizeof(uint64_t));
        m_terminal->Print("process exitted with code %i\n", exitCode);
        m_application = 0;
        PrintCommandHeader();
    }
}

void ResolverUnix::WaitForProcess()
{
    int      exitCode = 0;
    uint64_t value;

    wait(&exitCode);

    value = exitCode;
    write(m_processEvent, &value, sizeof(uint64_t));
}

bool ResolverUnix::ExecuteProgram(const std::string& Program, const std::vector<std::string>& Arguments)
{
    std::string line = "";

    // Set arguments
    if (Arguments.size() != 0) {
        for (unsigned int i = 0; i < Arguments.size(); i++) {
            if (i != 0) {
                line += " ";
            }
            line += Arguments[i];
        }
    }
    
    // God i hate forking
    auto rrStdin  = m_stdinFds[PIPE_READ];
    auto rrStdout = m_stdoutFds[PIPE_WRITE];
    auto rrStderr = m_stderrFds[PIPE_WRITE];
    auto childPid = fork();
    if (!childPid) {
        if (dup2(rrStdin, STDIN_FILENO)   == -1 ||
            dup2(rrStdout, STDOUT_FILENO) == -1 ||
            dup2(rrStderr, STDERR_FILENO) == -1) {
            exit(errno);
        }

        auto resultCode = execl(Program.c_str(), line.c_str(), NULL);
        exit(resultCode);
    }
    
    m_application = childPid;
    if (m_application != 0) {
        std::thread spawn(std::bind(&ResolverUnix::WaitForProcess, this));
        spawn.detach();
        return true;
    }
    return false;
}

std::vector<std::string> ResolverUnix::GetDirectoryContents(const std::string& Path)
{
    DIR*           dir;
    struct dirent* entry;
    std::vector<std::string> entries;

    dir = opendir(Path.c_str());
    if (dir != nullptr) {
        entry = readdir(dir);
        while (entry != NULL) {
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
                continue;
            }

            entries.push_back(std::string(entry->d_name));
            entry = readdir(dir);
        }
        closedir(dir);
    }

    std::sort(entries.begin(), entries.end());
    return entries;
}

bool ResolverUnix::IsProgramPathValid(const std::string& Path)
{
    struct stat stats;

    if (!stat(Path.c_str(), &stats)) {
        if (!S_ISREG(stats.st_mode)) {
            m_terminal->Print("%s: not an executable file\n", Path.c_str());
            return false;
        }

        if (access(Path.c_str(), X_OK)) {
            m_terminal->Print("%s: you do not have permissions to execute this file\n", Path.c_str());
            return false;
        }
        return true;
    }
    return false;
}

bool ResolverUnix::CommandResolver(const std::string& Command, const std::vector<std::string>& Arguments)
{
    std::string ProgramName = Command;
    std::string ProgramPath = "";

    ProgramPath = ProgramName;
    if (!IsProgramPathValid(ProgramPath)) {
        char TempBuffer[PATH_MAX] = { 0 };
        if (getcwd(&TempBuffer[0], PATH_MAX - 1) != nullptr) {
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

bool ResolverUnix::ListDirectory(const std::vector<std::string>& Arguments)
{
    auto path = m_currentDirectory;
    if (Arguments.size() != 0) {
        path = Arguments[0];
    }

    auto         directoryEntries = GetDirectoryContents(path);
    unsigned int longestEntry = 0;
    for (const auto& entry : directoryEntries) {
        if (entry.size() > longestEntry) {
            longestEntry = entry.size();
        }
    }

    // account for a space after each entry
    unsigned int entriesPerLine = m_terminal->GetNumberOfCellsPerLine() / (longestEntry + 1);
    for (unsigned int i = 0; i < directoryEntries.size(); i++) {
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

bool ResolverUnix::ChangeDirectory(const std::vector<std::string>& Arguments)
{
    if (Arguments.size() != 0) {
        std::string Path = Arguments[0];
        if (!chdir(Path.c_str())) {
            UpdateWorkingDirectory();
            return true;
        }
        m_terminal->Print("cd: invalid argument %s\n", Path.c_str());
        return false;
    }
    m_terminal->Print("cd: no argument given\n");
    return false;
}
