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
 * Terminal Implementation (Alumnious)
 * - The terminal emulator implementation for Vali. Built on manual rendering and
 *   using freetype as the font renderer.
 */

#include "terminal.hpp"
#include <drawing/font_manager.hpp>

#ifdef MOLLENOS
#include "targets/resolver_vali.hpp"
#include <io.h>
#include <ioctl.h>
#include <ioset.h>

#elif defined(_WIN32)

#else
#include "targets/resolver_unix.hpp"
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#endif

int main(int argc, char **argv)
{
    std::string                             fontPath = DATA_DIRECTORY "/fonts/DejaVuSansMono.ttf";
    std::shared_ptr<Asgaard::Drawing::Font> font     = Asgaard::Drawing::FM.CreateFont(fontPath, 12);
    Asgaard::Rectangle                      initialSize(-1, -1, 600, 400);

    std::string appGuid = "e9247a27-884e-4e27-a26a-19e5a6c57760";
    Asgaard::APP.SetSettingString(Asgaard::Application::Settings::APPLICATION_GUID, appGuid);

    // set pipes non-blocking
#ifdef MOLLENOS
    int  stdoutPipe = pipe(0x1000, 0);
    int  stderrPipe = pipe(0x1000, 0);
    auto resolver = std::make_shared<ResolverVali>(stdoutPipe, stderrPipe);
    int  opt = 1;

    ioctl(stdoutPipe, FIONBIO, &opt);
    ioctl(stderrPipe, FIONBIO, &opt);

    // initialize application
    Asgaard::APP.Initialize();

    // set the resolver's event descriptor
    resolver->Setup(resolver);
    
    auto window = Asgaard::APP.GetScreen()->CreateWindow<Terminal>(initialSize, 
        std::move(font), std::move(resolver), stdoutPipe, stderrPipe);
    Asgaard::APP.AddEventDescriptor(stdoutPipe, IOSETIN | IOSETLVT, window);
    Asgaard::APP.AddEventDescriptor(stderrPipe, IOSETIN | IOSETLVT, window);

    // We only call exit() to get out, so release ownership of window
    window.reset();

    return Asgaard::APP.Execute();
#elif defined(_WIN32)
    // initialize application
    Asgaard::APP.Initialize();

    // set the resolver's event descriptor
    resolver->Setup(resolver);
    
    auto window = Asgaard::APP.GetScreen()->CreateWindow<Terminal>(initialSize, 
        std::move(font), std::move(resolver), stdoutPipe, stderrPipe);
    Asgaard::APP.AddEventDescriptor(stdoutPipe, IOSETIN | IOSETLVT, window);
    Asgaard::APP.AddEventDescriptor(stderrPipe, IOSETIN | IOSETLVT, window);

    // We only call exit() to get out, so release ownership of window
    window.reset();

    return Asgaard::APP.Execute();
#else
    int stdinPipe[2];
    int stdoutPipe[2];
    int stderrPipe[2];
    int status;
    int opt = 1;

    status = pipe(stdinPipe);
    if (status < 0) {
        perror("allocating pipe for child input redirect");
        return status;
    }

    status = pipe(stdoutPipe);
    if (status < 0) {
        perror("allocating pipe for child output redirect");
        close(stdinPipe[PIPE_READ]);
        close(stdoutPipe[PIPE_WRITE]);
        return status;
    }
    
    status = pipe(stderrPipe);
    if (status < 0) {
        perror("allocating pipe for child output redirect");
        close(stdinPipe[PIPE_READ]);
        close(stdoutPipe[PIPE_WRITE]);
        close(stderrPipe[PIPE_WRITE]);
        return status;
    }
    
    auto resolver = std::make_shared<ResolverUnix>(stdoutPipe, stderrPipe, stdinPipe);
    ioctl(stdoutPipe[PIPE_READ], FIONBIO, &opt);
    ioctl(stderrPipe[PIPE_READ], FIONBIO, &opt);

    // initialize application
    Asgaard::APP.Initialize();

    auto window = Asgaard::APP.GetScreen()->CreateWindow<Terminal>(initialSize, 
        std::move(font), std::move(resolver), stdoutPipe[PIPE_READ], stderrPipe[PIPE_READ]);
    Asgaard::APP.AddEventDescriptor(stdoutPipe[PIPE_READ], EPOLLIN, window);
    Asgaard::APP.AddEventDescriptor(stderrPipe[PIPE_READ], EPOLLIN, window);

    // We only call exit() to get out, so release ownership of window
    window.reset();

    return Asgaard::APP.Execute();
#endif
}
