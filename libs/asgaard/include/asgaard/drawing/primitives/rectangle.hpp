/* ValiOS
 *
 * Copyright 2020, Philip Meulengracht
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

#include "shape.hpp"

namespace Asgaard
{
    namespace Drawing
    {
        namespace Primitives
        {
            class RectangleShape : public Shape {
            public:
                RectangleShape(int x, int y, int width, int height)
                    : Shape(ShapeType::RECTANGLE)
                    , m_x(x), m_y(y), m_width(width), m_height(height) { }

                int X() const { return m_x; }
                int Y() const { return m_y; }
                int Width() const { return m_width; }
                int Height() const { return m_height; }

            private:
                int m_x;
                int m_y;
                int m_width;
                int m_height;
            };
        }
    }
}
