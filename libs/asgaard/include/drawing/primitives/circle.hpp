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
            class CircleShape : public Shape {
            public:
                CircleShape(int centerX, int centerY, int radius)
                    : Shape(ShapeType::CIRCLE)
                    , m_centerX(centerX), m_centerY(centerY), m_radius(radius) { }

                int CenterX() const { return m_centerX; }
                int CenterY() const { return m_centerY; }
                int Radius() const{ return m_radius; }

            private:
                int m_centerX;
                int m_centerY;
                int m_radius;
            };
        }
    }
}
