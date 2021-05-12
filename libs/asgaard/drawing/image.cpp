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

#include "../include/drawing/color.hpp"
#include "../include/drawing/image.hpp"
#include "../include/exceptions/out_of_range_exception.h"
#include "../include/exceptions/application_exception.h"
#include <vector>
#include <iterator>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static std::string ExtendFilename(std::string& path, std::string& extension)
{
    if (extension == "") {
        return path;
    }

    auto lastTokenPos = path.find_last_of("/\\");
    auto lastDotPos = path.find_last_of(".");
    if (lastDotPos == std::string::npos) {
        // filepath with no extension 
        return path + extension;
    }
    else {
        auto fileExtension = path.substr(lastDotPos);

        if (lastTokenPos != std::string::npos) {
            if (lastDotPos > lastTokenPos) {
                // seperator comes before the last '.', which means the file has an extension
                return path.substr(0, lastDotPos) + extension + fileExtension;
            }
            else {
                // the dot is in a previous path token, which means no file extension
                return path + extension;
            }
        }

        // there is a dot but no seperator
        return path.substr(0, lastDotPos) + extension + fileExtension;
    }
}

namespace Asgaard {
    namespace Drawing {
        Image::Image(const std::string& path)
        {    
            int width;
            int height;
            int numComponents;

            auto buffer = stbi_load(path.c_str(), &width, &height, &numComponents, STBI_rgb_alpha);
            if (buffer == nullptr) {
                ZeroImage();
                return;
            }
            
            m_data = buffer;
            m_format = PixelFormat::A8B8G8R8; // images are loaded as rgba
            m_columns = width;
            m_rows = height;
            m_ownsBuffer = true;
        }

        Image::Image(std::istream& stream, std::size_t count)
        {
            std::vector<char> v;
            int               width;
            int               height;
            int               numComponents;
            
            auto readIntoVector = [&]
            {
                char* buffer = new char[count];
                stream.read(buffer, count);
                if (!stream)
                    throw ApplicationException("failed to read image stream", -1);
                std::copy(buffer, buffer + count, std::back_inserter(v));
                delete[] buffer;
            };

            readIntoVector();
            auto buffer = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(v.data()), v.size(), 
                &width, &height, &numComponents, STBI_rgb_alpha);
            if (buffer == nullptr) {
                ZeroImage();
                return;
            }

            m_data = buffer;
            m_format = PixelFormat::A8B8G8R8; // images are loaded as rgba
            m_columns = width;
            m_rows = height;
            m_ownsBuffer = true;
        }

        Image::Image(const void* imageData, PixelFormat format, int rows, int columns, bool takeOwnership)
            : m_data(const_cast<void*>(imageData))
            , m_format(format)
            , m_rows(rows)
            , m_columns(columns)
            , m_ownsBuffer(takeOwnership)
        {

        }

        Image::Image(const void* imageData, PixelFormat format, int rows, int columns)
            : Image(imageData, format, rows, columns, false) { }

        Image::Image()
        {
            ZeroImage();
        }

        Image::Image(int width, int height, PixelFormat format)
        {
            if (width > 0 && height > 0) {
                m_format = format;
                m_rows = height;
                m_columns = width;

                m_data = new char[Stride() * height];
                if (!m_data) {
                    // throw
                }
                m_ownsBuffer = true;

                memset(m_data, 0, Stride() * height);
            }
            else {
                ZeroImage();
            }
        }

        Image::Image(const Image& image)
        {
            // make a deep copy, and start with the allocation as that
            // can fail, so if we fail here we leave everything in ok order.
            m_data = new char[image.Stride() * image.m_rows];
            memcpy(m_data, image.m_data, image.Stride() * image.m_rows);
            m_ownsBuffer = true;

            // copy the rest of the data
            m_format = image.m_format;
            m_rows = image.m_rows;
            m_columns = image.m_columns;
        }

        Image::~Image()
        {
            if (m_data && m_ownsBuffer) {
                free(m_data);
            }
        }

        Image& Image::operator=(const Image& image)
        {
            // make a deep copy, and start with the allocation as that
            // can fail, so if we fail here we leave everything in ok order.
            m_data = new char[image.Stride() * image.m_rows];
            memcpy(m_data, image.m_data, image.Stride() * image.m_rows);
            m_ownsBuffer = true;
            
            // copy the rest of the data
            m_format = image.m_format;
            m_rows = image.m_rows;
            m_columns = image.m_columns;

            return *this;
        }

        Color Image::GetPixel(int index) const
        {
            auto pointer = static_cast<uint8_t*>(m_data);
            int  byteIndex = index * GetBytesPerPixel(m_format);
            if (byteIndex >= Stride() * m_rows) {
                throw OutOfRangeException("Image::GetPixel(index) was out of range");
            }

            auto colorPointer = reinterpret_cast<uint32_t*>(&pointer[byteIndex]);
            return Color::FromFormatted(*colorPointer, m_format);
        }

        void Image::SetPixel(int index, const Color& color)
        {
            auto pointer = static_cast<uint8_t*>(m_data);
            int  byteIndex = index * GetBytesPerPixel(m_format);
            if (byteIndex >= Stride() * m_rows) {
                throw OutOfRangeException("Image::GetPixel(index) was out of range");
            }

            auto colorPointer = reinterpret_cast<uint32_t*>(&pointer[byteIndex]);
            *colorPointer = color.GetFormatted(m_format);
        }

        void Image::ZeroImage()
        {
            m_data = nullptr;
            m_format = PixelFormat::A8R8G8B8;
            m_rows = 0;
            m_columns = 0;
            m_ownsBuffer = false;
        }
    }
}