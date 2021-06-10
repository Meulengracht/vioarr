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

#include "../pixel_format.hpp" 

namespace Asgaard {
    namespace Drawing {
        class Color {
        public:
            Color(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
                : m_a(a), m_r(r), m_g(g), m_b(b) { }
            Color(unsigned char r, unsigned char g, unsigned char b)
                : Color(0xFF, r, g, b) { }
            Color()
                : Color(0xFF, 0, 0, 0) { }

        unsigned int GetFormatted(PixelFormat format) const
        {
            return Format(m_a, m_r, m_g, m_b, format);
        }

        void          SetAlpha(unsigned char a) { m_a = a; }
        void          SetRed(unsigned char r)   { m_r = r; }
        void          SetGreen(unsigned char g) { m_g = g; }
        void          SetBlue(unsigned char b)  { m_b = b; }
        unsigned char Alpha() const             { return m_a; }
        unsigned char Red()   const             { return m_r; }
        unsigned char Green() const             { return m_g; }
        unsigned char Blue()  const             { return m_b; }

        public:
            static Color FromFormatted(unsigned int color, PixelFormat format)
            {
                union {
                    struct {
                        unsigned char c0;
                        unsigned char c1;
                        unsigned char c2;
                        unsigned char c3;
                    } components;
                    unsigned int color;
                } colorData;

                colorData.color = color;

                switch (format) {
                    case PixelFormat::A8R8G8B8: return Color(colorData.components.c3, colorData.components.c2, colorData.components.c1, colorData.components.c0);
                    case PixelFormat::A8B8G8R8: return Color(colorData.components.c3, colorData.components.c0, colorData.components.c1, colorData.components.c2);
                    case PixelFormat::X8R8G8B8: return Color(colorData.components.c2, colorData.components.c1, colorData.components.c0);
                    case PixelFormat::R8G8B8A8: return Color(colorData.components.c0, colorData.components.c3, colorData.components.c2, colorData.components.c1);
                    case PixelFormat::B8G8R8A8: return Color(colorData.components.c0, colorData.components.c1, colorData.components.c2, colorData.components.c3);
                    case PixelFormat::X8B8G8R8: return Color(colorData.components.c0, colorData.components.c1, colorData.components.c2);
                }
                return Color(0, 0, 0);
            }

            static unsigned int Format(unsigned char a, unsigned char r, unsigned char g, unsigned char b, PixelFormat format)
            {
                unsigned int color = 0;
                switch (format) {
                    case PixelFormat::A8R8G8B8:
                        color = ((unsigned int)a << 24) | ((unsigned int)r << 16) | ((unsigned int)g << 8) | b;
                        break;
                    case PixelFormat::A8B8G8R8:
                        color = ((unsigned int)a << 24) | ((unsigned int)b << 16) | ((unsigned int)g << 8) | r;
                        break;
                    case PixelFormat::X8R8G8B8:
                        color = 0xFF000000 | ((unsigned int)r << 16) | ((unsigned int)g << 8) | b;
                        break;
                    case PixelFormat::R8G8B8A8:
                        color = ((unsigned int)r << 24) | ((unsigned int)g << 16) | ((unsigned int)b << 8) | a;
                        break;
                    case PixelFormat::B8G8R8A8:
                        color = ((unsigned int)b << 24) | ((unsigned int)g << 16) | ((unsigned int)r << 8) | a;
                        break;
                    case PixelFormat::X8B8G8R8:
                        color = 0xFF000000 | ((unsigned int)b << 16) | ((unsigned int)g << 8) | r;
                        break;
                }
                return color;
            }

        private:
            unsigned char m_a, m_r, m_g, m_b;
        };
    }
}