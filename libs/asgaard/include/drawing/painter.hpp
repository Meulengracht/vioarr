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

#include <memory>
#include "../config.hpp"
#include "color.hpp"

namespace Asgaard {
    class MemoryBuffer;
    class Rectangle;
    
    namespace Drawing {
        class Font;
        class Image;
        
        class ASGAARD_API Painter {
            public:
                Painter(const std::shared_ptr<MemoryBuffer>& canvas);
                
                void SetFillColor(const Color& color);
                void SetFillColor(unsigned char a, unsigned char r, unsigned char g, unsigned char b);
                void SetFillColor(unsigned char r, unsigned char g, unsigned char b);

                void SetOutlineColor(const Color& color);
                void SetOutlineColor(unsigned char a, unsigned char r, unsigned char g, unsigned char b);
                void SetOutlineColor(unsigned char r, unsigned char g, unsigned char b);

                void RenderLine(int x1, int y1, int x2, int y2);
                
                void RenderFillGradientV(const Rectangle& dimensions,
                    unsigned char r1, unsigned char g1, unsigned char b1,
                    unsigned char r2, unsigned char g2, unsigned char b2);
                void RenderFillGradientV(
                    unsigned char r1, unsigned char g1, unsigned char b1,
                    unsigned char r2, unsigned char g2, unsigned char b2);

                void RenderFill(const Rectangle& dimensions);
                void RenderFill();

                void RenderImage(const Image& image);
                
                void SetFont(const std::shared_ptr<Font>& font);
                
                void RenderCharacter(int x, int y, char character);
                void RenderText(const Rectangle& dimensions, const std::string& text);
                void RenderText(int x, int y, const std::string& text);
                
            private:
                std::shared_ptr<MemoryBuffer> m_canvas;
                std::shared_ptr<Font>         m_font;
                Color                         m_fillColor;
                Color                         m_outlineColor;
        };
    }
}
