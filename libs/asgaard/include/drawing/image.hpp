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
 * ValiOS - Application Framework (Asgaard)
 *  - Contains the implementation of the application framework used for building
 *    graphical applications.
 */
#pragma once

#include "../config.hpp"
#include "../pixel_format.hpp"
#include "color.hpp"

#include <string>
#include <istream>

namespace Asgaard {
    namespace Drawing {
        class ASGAARD_API Image {
        public:
            Image();
            Image(int width, int height, PixelFormat format);
            Image(const Image&);
            Image(const std::string& path);
            Image(std::istream&, std::size_t);
            Image(const void* imageData, PixelFormat format, int rows, int columns);
            Image(const void* imageData, PixelFormat format, int rows, int columns, bool takeOwnership);
            ~Image();

            Color GetPixel(int index) const;
            void  SetPixel(int index, const Color& color);

            int Width()  const { return m_columns; }
            int Height() const { return m_rows; }
            int Stride() const { return m_columns * GetBytesPerPixel(m_format); }
            PixelFormat Format() const { return m_format; }
            void* Data() const { return m_data; }


            Image& operator=(const Image&);

        private:
            void ZeroImage();

        private:
            void*       m_data;
            PixelFormat m_format;
            int         m_rows;
            int         m_columns;
            bool        m_ownsBuffer;
        };
    }
}
