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

#include <exception>
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
#include <pwd.h>
#include <cstring>

namespace {
    std::string ResolveProfileName()
    {
        auto* pw = getpwuid(geteuid());
        if (pw)
        {
            return std::string(pw->pw_name);
        }
        return {};
    }
}

ResolverUnix::ResolverUnix(const int* stdoutFds, const int* stderrFds, const int* stdinFds)
    : ResolverBase()
    , m_profile(ResolveProfileName())
    , m_application(-1)
    , m_stdoutFds{stdoutFds[0], stdoutFds[1]}
    , m_stdinFds{stdinFds[0], stdinFds[1]}
    , m_stderrFds{stderrFds[0], stderrFds[1]}
    , m_processEvent(-1)
{
    UpdateWorkingDirectory();
}

ResolverUnix::~ResolverUnix()
{
    if (m_processEvent != -1) {
        close(m_processEvent);
    }
}

void ResolverUnix::UpdateWorkingDirectory()
{
    char* CurrentPath = (char*)std::malloc(PATH_MAX);
    std::memset(CurrentPath, 0, PATH_MAX);
    if (getcwd(CurrentPath, PATH_MAX - 1) != nullptr) {
        m_currentDirectory = std::string(CurrentPath);
    }
    else {
        m_currentDirectory = "nil";
    }
    std::free(CurrentPath);
}

bool ResolverUnix::HandleKeyCode(const Asgaard::KeyEvent& key)
{
    if (m_application > 0) {
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
    return ResolverBase::HandleKeyCode(key);
}

void ResolverUnix::PrintCommandHeader()
{
    // Dont print the command header if an application is running
    if (m_application <= 0) {
        m_terminal->Print("\033[34m%s@%s~\033[39m ", m_profile.c_str(), m_currentDirectory.c_str());
    }
}

void ResolverUnix::DescriptorEvent(int iod, unsigned int events)
{
    if (iod == m_processEvent) {
        uint64_t exitCode;
        
        read(iod, &exitCode, sizeof(uint64_t));
        m_terminal->Print("process exitted with code %i\n", exitCode);
        m_application = -1;
        PrintCommandHeader();
    }
}

void ResolverUnix::WaitForProcess()
{
    int      exitCode = 0;
    uint64_t value;

    waitpid(m_application, &exitCode, 0);

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

    if (m_processEvent == -1) {
        m_processEvent = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        if (m_processEvent < 0) {
            throw std::logic_error("m_processEvent failed to be created");
        }

        Asgaard::APP.AddEventDescriptor(m_processEvent, 
            EPOLLERR | EPOLLIN | EPOLLET, 
            shared_from_this()
        );
    }
    
    // God i hate forking
    auto childPid = fork();
    if (!childPid) {
        if (dup2(m_stdinFds[PIPE_READ], STDIN_FILENO)   == -1 ||
            dup2(m_stdoutFds[PIPE_WRITE], STDOUT_FILENO) == -1 ||
            dup2(m_stderrFds[PIPE_WRITE], STDERR_FILENO) == -1) {
            exit(errno);
        }
        
        // don't need those anymore
        close(m_stdinFds[PIPE_READ]);
        close(m_stdinFds[PIPE_WRITE]);
        close(m_stdoutFds[PIPE_READ]);
        close(m_stdoutFds[PIPE_WRITE]);
        close(m_stderrFds[PIPE_READ]);
        close(m_stderrFds[PIPE_WRITE]);

        auto resultCode = execl(Program.c_str(), line.c_str(), NULL);
        exit(resultCode);
    }
    
    m_application = childPid;
    if (m_application > 0) {
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
                entry = readdir(dir);
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

bool ResolverUnix::ChangeDirectory(const std::vector<std::string>& arguments)
{
    if (arguments.size() != 0) {
        std::string path = arguments[0];
        if (!chdir(path.c_str())) {
            UpdateWorkingDirectory();
            return true;
        }
        m_terminal->Print("cd: invalid argument %s\n", path.c_str());
        return false;
    }
    m_terminal->Print("cd: no argument given\n");
    return false;
}
