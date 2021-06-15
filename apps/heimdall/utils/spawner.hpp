/**
 * Copyright 2021, Philip Meulengracht
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
 * ValiOS - Application Environment (Launcher)
 *  - Contains the implementation of the application environment to support
 *    graphical user interactions.
 */
#pragma once

#include <string>

#ifdef MOLLENOS
#include <os/process.h>
#else
#include <unistd.h>
#endif

class Spawner {
public:
    static void SpawnApplication(const std::string& path)
    {
#ifdef MOLLENOS
        UUId_t pid;
        ProcessSpawn(path.c_str(), NULL, &pid);
#else
        auto childPid = fork();
        if (!childPid) {
            char* argv[] = { const_cast<char*>(path.c_str()), NULL };
            auto  resultCode = execv(path.c_str(), argv);
            exit(resultCode);
        }
#endif
    }
};
