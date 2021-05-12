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

#include <io.h>
#include <ioctl.h>
#include <ioset.h>
#include "targets/resolver_vali.hpp"
#include "terminal.hpp"
#include <asgaard/drawing/font_manager.hpp>

int main(int argc, char **argv)
{
    std::string                             fontPath = "$sys/fonts/DejaVuSansMono.ttf";
    std::shared_ptr<Asgaard::Drawing::Font> font     = Asgaard::Drawing::FM.CreateFont(fontPath, 12);
    int                                     stdoutPipe = pipe(0x1000, 0);
    int                                     stderrPipe = pipe(0x1000, 0);
    std::shared_ptr<ResolverVali>           resolver = std::shared_ptr<ResolverVali>(new ResolverVali(stdoutPipe, stderrPipe));
    Asgaard::Rectangle                      initialSize(0, 0, 600, 400);

    // set pipes non-blocking
    int opt = 1;
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
}
