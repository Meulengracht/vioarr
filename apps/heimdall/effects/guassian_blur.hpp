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

#include <drawing/image.hpp>

class GuassianBlurEffect
{
public:
    Asgaard::Drawing::Image Apply(Asgaard::Drawing::Image& image, double radius);

private:
    int* CreateBoxesForGuass(double sigma, int boxCount);
    void BoxBlur(Asgaard::Drawing::Image& source, Asgaard::Drawing::Image& output, int radius);
    void BoxBlurH(Asgaard::Drawing::Image& source, Asgaard::Drawing::Image& output, int radius);
    void BoxBlurT(Asgaard::Drawing::Image& source, Asgaard::Drawing::Image& output, int radius);
};
