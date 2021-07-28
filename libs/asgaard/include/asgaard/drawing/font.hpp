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
#include "../rectangle.hpp"

namespace Asgaard {
    namespace Utils {
        class FreeType;
    }
    
    namespace Drawing {
        class ASGAARD_API Font {
        public:
            struct Glyph;
            typedef struct FT_FaceRec_* FT_Face;
            
            struct CharInfo {
                uint8_t*     bitmap;
                int          pitch;
                int          width;
                int          height;
                int          indentX;
                int          indentY;
                int          advance;
                unsigned int index;
            };
        public:
            Font(const std::shared_ptr<Utils::FreeType>& freetype, int pixelSize);
            ~Font();
            
            bool Initialize(const std::string& path);
            
            bool SetPixelSize(int pixelSize);
            bool GetCharacterBitmap(unsigned long character, struct CharInfo& bitmap);
            
            int       GetFontHeight() const { return m_height; }
            int       GetFontWidth() const { return m_fixedWidth; }
            Rectangle GetTextMetrics(const std::string& text);
            
        private:
            bool LoadGlyph(unsigned long character, struct Glyph* cached, int want);
            bool FindGlyph(unsigned long character, int want);
            void FlushGlyph(struct Glyph* glyph);
            void FlushCache();
            
        private:
            std::shared_ptr<Utils::FreeType> m_freetype;
            FT_Face                          m_face;
            bool                             m_valid;
        
            struct Glyph* m_current;
            struct Glyph* m_cache;
            
            // stored font metrics
            int          m_pixelSize;
            int          m_height;
            int          m_fixedWidth;
            int          m_ascent;
            int          m_descent;
            int          m_lineSkip;
            std::size_t  m_fontSizeFamily;
            int          m_faceStyle;
            int          m_style;
            int          m_outline;
            int          m_kerning;
            int          m_hinting;
            int          m_glyphOverhang;
            float        m_glyphItalics;
            int          m_underlineOffset;
            int          m_underlineHeight;
        };
    }
}
