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
 
#include <asgaard/drawing/painter.hpp>
#include <asgaard/drawing/font.hpp>
#include <asgaard/drawing/image.hpp>
#include <asgaard/memory_buffer.hpp>
#include <asgaard/rectangle.hpp>
#include <asgaard/drawing/primitives/rectangle.hpp>
#include <asgaard/drawing/primitives/circle.hpp>
#include <cstring>
#include <string>
#include <cmath>

namespace {
    unsigned int AlphaBlendAXGX(unsigned int colorA, unsigned int colorB, unsigned int alpha)
    {
        unsigned int rb1 = ((0x100 - alpha) * (colorA & 0xFF00FF)) >> 8;
        unsigned int rb2 = (alpha * (colorB & 0xFF00FF)) >> 8;
        unsigned int g1  = ((0x100 - alpha) * (colorA & 0x00FF00)) >> 8;
        unsigned int g2  = (alpha * (colorB & 0x00FF00)) >> 8;
        return (((rb1 | rb2) & 0xFF00FF) + ((g1 | g2) & 0x00FF00)) | 0xFF000000;
    }
}

#include <iostream>

/**
 * All these algorithms are naive and non-optimized algorithms. When we encounter
 * severe performance problems (and we will!). We must start implementing optimized versions.
 * This will also include SIMD accellerated versions.
 */
namespace Asgaard {
    namespace Drawing {
        Painter::Painter(const std::shared_ptr<MemoryBuffer>& canvas)
            : m_canvas(canvas)
            , m_font(nullptr)
            , m_fillColor(0, 0, 0)
            , m_outlineColor(0, 0, 0)
            , m_defaultShape(Primitives::RectangleShape(0, 0, canvas->Width(), canvas->Height()))
        {
            m_shape = &m_defaultShape;
        }
        
        void Painter::SetFillColor(const Color& color)
        {
            m_fillColor = color;
        }
        
        void Painter::SetFillColor(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
        {
            m_fillColor = Color(a, r, g, b);
        }
        
        void Painter::SetFillColor(unsigned char r, unsigned char g, unsigned char b)
        {
            SetFillColor(0xFF, r, g, b);
        }
        
        void Painter::SetOutlineColor(const Color& color)
        {
            m_outlineColor = color;
        }

        void Painter::SetOutlineColor(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
        {
            m_outlineColor = Color(a, r, g, b);
        }
        
        void Painter::SetOutlineColor(unsigned char r, unsigned char g, unsigned char b)
        {
            SetOutlineColor(0xFF, r, g, b);
        }
        
        void Painter::SetFont(const std::shared_ptr<Font>& font)
        {
            m_font = font;
        }

        void Painter::SetRegion(const Primitives::Shape* shape)
        {
            if (shape == nullptr) {
                m_shape = &m_defaultShape;
                return;
            }
            
            m_shape = shape;
        }

        void Painter::RenderLine(int x1, int y1, int x2, int y2)
        {
            uint32_t* pointer = static_cast<uint32_t*>(m_canvas->Buffer());
            int       dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
            int       dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
            int       err = (dx > dy ? dx : -dy) / 2, e2;

            unsigned int color = m_fillColor.GetFormatted(m_canvas->Format());
            while (x1 < m_canvas->Width() && y1 < m_canvas->Height()) {
                pointer[(y1 * m_canvas->Width()) + x1] = color;

                if (x1 == x2 && y1 == y2) {
                    break;
                }
                
                e2 = err;
                if (e2 > -dx) { err -= dy; x1 += sx; }
                if (e2 <  dy) { err += dx; y1 += sy; }
            }
        }

        void Painter::RenderRectangleFillGradientV(const Rectangle& dimensions,
                    unsigned char r1, unsigned char g1, unsigned char b1,
                    unsigned char r2, unsigned char g2, unsigned char b2)
        {
            Color originalColor = m_fillColor;
            for (int y = 0; y < dimensions.Height(); y++) {
                float p = y / (float)(dimensions.Height() - 1);
                unsigned char r = (unsigned char)((1.0f - p) * r1 + p * r2 + 0.5);
                unsigned char g = (unsigned char)((1.0f - p) * g1 + p * g2 + 0.5);
                unsigned char b = (unsigned char)((1.0f - p) * b1 + p * b2 + 0.5);
                SetFillColor(r, g, b);
                RenderLine(dimensions.X(), y, dimensions.X() + dimensions.Width(), y);
            }
            m_fillColor = originalColor;
        }
        
        void Painter::RenderRectangleFill(const Rectangle& dimensions)
        {
            for (int y = dimensions.Y(); y < (dimensions.Y() + dimensions.Height()); y++) {
                RenderLine(dimensions.X(), y, dimensions.X() + dimensions.Width(), y);
            }
        }
        
        void Painter::RenderRectangle(const Rectangle& dimensions)
        {
            auto left = dimensions.X();
            auto right = dimensions.X() + dimensions.Width() - 1;
            auto top = dimensions.Y();
            auto bottom = dimensions.Y() + dimensions.Height() - 1;

            RenderLine(
                left, top,
                right, top
            );
            
            RenderLine(
                left, top,
                left, bottom
            );

            RenderLine(
                left, bottom, 
                right, bottom
            );
            
            RenderLine(
                right, top, 
                right, bottom
            );
        }

        void Painter::RenderRectangle(int top, int left, int bottom, int right)
        {
            RenderLine(
                left, top,
                right, top
            );
            
            RenderLine(
                left, top,
                left, bottom
            );

            RenderLine(
                left, bottom, 
                right, bottom
            );
            
            RenderLine(
                right, top, 
                right, bottom
            );
        }

        void Painter::RenderCircleFill(int centerX, int centerY, int radius)
        {
            auto radiusSqrt = radius * radius;
            auto bgColor    = m_fillColor.GetFormatted(m_canvas->Format());

            for (int x = -radius; x < radius; x++)
            {
                auto height = (int)std::sqrt(radiusSqrt - x * x);
                for (int y = -height; y < height; y++) {
                    if (x + centerX < m_canvas->Width() && x + centerX >= 0 &&
                        y + centerY < m_canvas->Height() && y + centerY >= 0) {
                        *static_cast<uint32_t*>(m_canvas->Buffer(centerX + x, centerY + y)) = bgColor;
                    }
                }
            }
        }

        void Painter::RenderCircle(int centerX, int centerY, int radius)
        {
            int  x      = radius;
            int  y      = 0;
            auto mid    = static_cast<uint8_t*>(m_canvas->Buffer(centerX, centerY));
            auto slSize = m_canvas->Stride();
            auto color  = m_outlineColor.GetFormatted(m_canvas->Format());
            auto bpp    = GetBytesPerPixel(m_canvas->Format());

            auto plot = [&] (const int pX, const int pY) {
                if (pX < m_canvas->Width() && pX >= 0 &&
                    pY < m_canvas->Height() && pY >= 0) {
                    int offset = ((slSize * pY) + (pX * bpp));
                    *(reinterpret_cast<uint32_t*>(mid + offset)) = color;
                }
            };

            plot(centerX + x, centerY + y);
            if (radius > 0) {
                plot(x + centerX, -y + centerY);
                plot(y + centerX, x + centerY);
                plot(-y + centerX, x + centerY);
            }

            auto P = 1 - radius;
            while (x > y)
            { 
                y++;
                
                // Mid-point is inside or on the perimeter
                if (P <= 0) {
                    P = P + 2*y + 1;
                }
                else // Mid-point is outside the perimeter
                {
                    x--;
                    P = P + 2*y - 2*x + 1;
                }
                
                // All the perimeter points have already been printed
                if (x < y) {
                    break;
                }
                
                plot(x + centerX, y + centerY);
                plot(-x + centerX, y + centerY);
                plot(x + centerX, -y + centerY);
                plot(-x + centerX, -y + centerY);
                
                // If the generated point is on the line x = y then 
                // the perimeter points have already been printed
                if (x != y)
                {
                    plot(y + centerX, x + centerY);
                    plot(-y + centerX, x + centerY);
                    plot(y + centerX, -x + centerY);
                    plot(-y + centerX, -x + centerY);
                }
            }
        }

        void Painter::RenderFill()
        {
            switch (m_shape->GetType()) {
                case Primitives::ShapeType::RECTANGLE: {
                    const auto rectangle = static_cast<const Primitives::RectangleShape*>(m_shape);
                    auto left = rectangle->X();
                    auto right = rectangle->X() + rectangle->Width();
                    for (int y = 0; y < rectangle->Height(); y++) {
                        RenderLine(
                            left,
                            rectangle->Y() + y,
                            right,
                            rectangle->Y() + y
                        );
                    }
                } break;
                case Primitives::ShapeType::CIRCLE: {
                    const auto circle = static_cast<const Primitives::CircleShape*>(m_shape);
                    RenderCircleFill(circle->CenterX(), circle->CenterY(), circle->Radius());
                } break;
            }
        }

        void Painter::RenderFillGradientV(
                    unsigned char r1, unsigned char g1, unsigned char b1,
                    unsigned char r2, unsigned char g2, unsigned char b2)
        {
            Color originalColor = m_fillColor;
            for (int y = 0; y < m_canvas->Height(); y++) {
                float p = y / (float)(m_canvas->Height() - 1);
                unsigned char r = (unsigned char)((1.0f - p) * r1 + p * r2 + 0.5);
                unsigned char g = (unsigned char)((1.0f - p) * g1 + p * g2 + 0.5);
                unsigned char b = (unsigned char)((1.0f - p) * b1 + p * b2 + 0.5);
                SetFillColor(r, g, b);
                RenderLine(0, y, m_canvas->Width(), y);
            }
            m_fillColor = originalColor;
        }

        void Painter::RenderImage(int x, int y, const Image& image)
        {
            // if faulty images are provided we just return
            if (image.Width() == 0 || image.Height() == 0) {
                return;
            }

            auto bpp = GetBytesPerPixel(m_canvas->Format());
            switch (m_shape->GetType()) {
                case Primitives::ShapeType::RECTANGLE: {
                    const auto rectangle      = static_cast<const Primitives::RectangleShape*>(m_shape);
                    auto destination          = static_cast<uint32_t*>(m_canvas->Buffer(rectangle->X(), rectangle->Y()));
                    auto bytesSkipSource      = x * bpp;
                    auto strideDestination    = rectangle->Width() * bpp;
                    auto bytesLeft            = std::min(image.Stride() - bytesSkipSource, strideDestination);
                    auto pixelsLeft           = bytesLeft / bpp;
                    auto fillcolor            = m_fillColor.GetFormatted(m_canvas->Format());

                    for (auto i = 0; i < std::min(image.Height() - y, rectangle->Height()); i++) {
                        for (auto j = 0; j < pixelsLeft; j++) {
                            auto pixel = image.GetPixel((i + y) * image.Width() + (x + j));
                            if (pixel.Alpha() == 255) {
                                *(destination + j) = pixel.GetFormatted(m_canvas->Format());
                            }
                            else if (pixel.Alpha() > 0) {
                                *(destination + j) = AlphaBlendAXGX(
                                    fillcolor, pixel.GetFormatted(m_canvas->Format()), pixel.Alpha()
                                );
                            }
                        }

                        destination += m_canvas->Width();
                    }
                } break;
                case Primitives::ShapeType::CIRCLE: {
                    const auto circle = static_cast<const Primitives::CircleShape*>(m_shape);
                    auto radiusSqrt = circle->Radius() * circle->Radius();
                    auto fillcolor  = m_fillColor.GetFormatted(m_canvas->Format());
                    auto midImage   = ((image.Height() >> 1) * image.Width()) + (image.Width() >> 1);
                    auto imageLimit = image.Height() * image.Width();

                    for (int x = -circle->Radius(); x < circle->Radius(); x++)
                    {
                        auto height = (int)std::sqrt(radiusSqrt - x * x);
                        for (int y = -height; y < height; y++) {
                            auto imageOffset = midImage + (y * image.Width()) + x;
                            if (x + circle->CenterX() < m_canvas->Width() && x + circle->CenterX() >= 0 &&
                                y + circle->CenterY() < m_canvas->Height() && y + circle->CenterY() >= 0 &&
                                imageOffset >= 0 && imageOffset < imageLimit) {
                                auto dest = static_cast<uint32_t*>(m_canvas->Buffer(circle->CenterX() + x, circle->CenterY() + y));
                                auto pixel = image.GetPixel(imageOffset);
                                if (pixel.Alpha() == 255) {
                                    *dest = pixel.GetFormatted(m_canvas->Format());
                                }
                                else if (pixel.Alpha() > 0) {
                                    *dest = AlphaBlendAXGX(fillcolor,
                                        pixel.GetFormatted(m_canvas->Format()), 
                                        pixel.Alpha()
                                    );
                                }
                            }
                        }
                    }
                } break;
            }
        }

        void Painter::RenderImage(const Image& image)
        {
            RenderImage(0, 0, image);
        }
        
        void Painter::RenderCharacter(int x, int y, char character)
        {
            struct Font::CharInfo bitmap = { 0 };
            
            if (!m_font) {
                return;
            }

            unsigned int bgColor = m_fillColor.GetFormatted(m_canvas->Format());
            unsigned int fgColor = m_outlineColor.GetFormatted(m_canvas->Format());
            if (m_font->GetCharacterBitmap(character, bitmap)) {
                uint32_t* pointer = static_cast<uint32_t*>(m_canvas->Buffer(
                    x + bitmap.indentX, y + bitmap.indentY));
                uint8_t*  source  = bitmap.bitmap;
                for (int row = 0; row < bitmap.height; row++) {
                    for (int column = 0; column < bitmap.width; column++) {
                        uint8_t alpha = source[column];
                        if (alpha == 255) {
                            pointer[column] = fgColor;
                        }
                        else if (alpha == 0) {
                            pointer[column] = bgColor;
                        }
                        else {
                            // ok so we must blend the text color, but with what?
                            if (m_fillColor.Alpha() == 255) {
                                pointer[column] = AlphaBlendAXGX(bgColor, fgColor, alpha);
                            }
                            else {
                                pointer[column] = Color::Format(alpha, 
                                    m_outlineColor.Red(), 
                                    m_outlineColor.Green(), 
                                    m_outlineColor.Blue(),
                                    m_canvas->Format()
                                );
                            }
                        }
                    }
                    
                    pointer = reinterpret_cast<uint32_t*>(reinterpret_cast<std::uintptr_t>(pointer) + m_canvas->Stride());
                    source  += bitmap.pitch;
                }
            }
        }
        
        void Painter::RenderText(const Rectangle& dims, const std::string& text)
        {
            struct Font::CharInfo bitmap   = { 0 };
            int                   currentX = dims.X();
            int                   currentY = dims.Y();
            
            if (!m_font) {
                return;
            }
            
            auto bgColor = m_fillColor.GetFormatted(m_canvas->Format());
            auto fgColor = m_outlineColor.GetFormatted(m_canvas->Format());
            for (auto i = 0u; i < text.length(); i++) {
                auto character = text[i];
                
                if (m_font->GetCharacterBitmap(character, bitmap)) {
                    if ((currentX + bitmap.indentX + bitmap.advance) >= dims.Width()) {
                        currentY += m_font->GetFontHeight();
                        currentX  = dims.X();
                    }

                    if ((currentY + m_font->GetFontHeight()) > dims.Height()) {
                        break;
                    }
                    
                    uint32_t* pointer = static_cast<uint32_t*>(m_canvas->Buffer(
                        currentX + bitmap.indentX, currentY + bitmap.indentY));
                    uint8_t*  source  = bitmap.bitmap;
                    
                    for (int row = 0; row < bitmap.height; row++) {
                        for (int column = 0; column < bitmap.width; column++) {
                            uint8_t alpha = source[column];
                            if (alpha == 255) {
                                pointer[column] = fgColor;
                            }
                            else if (alpha == 0) {
                                pointer[column] = bgColor;
                            }
                            else {
                                // ok so we must blend the text color, but with what?
                                if (m_fillColor.Alpha() == 255) {
                                    pointer[column] = AlphaBlendAXGX(bgColor, fgColor, alpha);
                                }
                                else {
                                    pointer[column] = Color::Format(alpha, 
                                        m_outlineColor.Red(), 
                                        m_outlineColor.Green(), 
                                        m_outlineColor.Blue(),
                                        m_canvas->Format()
                                    );
                                }
                            }
                        }
                        
                        pointer = reinterpret_cast<uint32_t*>(reinterpret_cast<std::uintptr_t>(pointer) + m_canvas->Stride());
                        source  += bitmap.pitch;
                    }
                }
                
                currentX += bitmap.indentX + bitmap.advance;
            }
        }
        
        void Painter::RenderText(int x, int y, const std::string& text)
        {
            Rectangle dimensions(x, y, m_canvas->Width(), m_canvas->Height());
            RenderText(dimensions, text);
        }
    }
}
