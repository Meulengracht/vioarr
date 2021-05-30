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
 * ValiOS - Application Framework (Asgaard)
 *  - Contains the implementation of the application framework used for building
 *    graphical applications.
 */
#pragma once

#if defined(WINDOWS) || defined(_WIN32) || defined(MOLLENOS)
#  ifdef ASGAARD_BUILD
#    define ASGAARD_API __declspec(dllexport)
#  else
#    define ASGAARD_API __declspec(dllimport)
#  endif
#endif

#ifndef ASGAARD_API
#define ASGAARD_API
#endif

#if defined(MOLLENOS)
#define DATA_DIRECTORY "$sys"
#elif defined(_WIN32)
#define DATA_DIRECTORY "."
#else
#define DATA_DIRECTORY "."
#endif
