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
 
#include <cassert>
#include <cstdio>
#include <cctype>
#include <cmath>
#include "../include/utils/freetype.hpp"
#include "../include/drawing/font.hpp"
#include <fstream>

/* FIXME: Right now we assume the gray-scale renderer Freetype is using
          supports 256 shades of gray, but we should instead key off of num_grays
          in the result FT_Bitmap after the FT_Render_Glyph() call. */
#define NUM_GRAYS 256

/* Handy routines for converting from fixed point */
#define FT_FLOOR(x) ((x & -64) / 64)
#define FT_CEIL(x)  (((x + 63) & -64) / 64)

/* Set and retrieve the font style */
#define TTF_STYLE_NORMAL        0x00
#define TTF_STYLE_BOLD          0x01
#define TTF_STYLE_ITALIC        0x02
#define TTF_STYLE_UNDERLINE     0x04
#define TTF_STYLE_STRIKETHROUGH 0x08

/* Handle a style only if the font does not already handle it */
#define TTF_HANDLE_STYLE_BOLD(font) (((font)->m_style & TTF_STYLE_BOLD) && \
                                    !((font)->m_faceStyle & TTF_STYLE_BOLD))
#define TTF_HANDLE_STYLE_ITALIC(font) (((font)->m_style & TTF_STYLE_ITALIC) && \
                                      !((font)->m_faceStyle & TTF_STYLE_ITALIC))
#define TTF_HANDLE_STYLE_UNDERLINE(font) ((font)->m_style & TTF_STYLE_UNDERLINE)
#define TTF_HANDLE_STYLE_STRIKETHROUGH(font) ((font)->m_style & TTF_STYLE_STRIKETHROUGH)

#define CACHED_METRICS  0x10
#define CACHED_BITMAP   0x01
#define CACHED_PIXMAP   0x02

#define CACHE_SIZE 257

namespace {
    bool LoadFile(const std::string& path, char** baseOut, size_t* sizeOut)
    {
        std::ifstream fs;
        
        fs.open(path, std::ios::in | std::ios::binary | std::ios::ate);
        if (fs.is_open()) {
            *sizeOut = (size_t)fs.tellg();
            if (*sizeOut != 0) {
                fs.seekg(0, std::ios_base::beg);
                *baseOut = (char*)std::malloc(*sizeOut);
                if (*baseOut != nullptr) {
                    fs.read(*baseOut, *sizeOut);
                    return true;
                }
            }
        }
        return false;
    }
}

namespace Asgaard {
    namespace Drawing {
        struct Font::Glyph {
            int           stored;
            unsigned int  index;
            FT_Bitmap     bitmap;
            FT_Bitmap     pixmap;
            int           minX;
            int           maxX;
            int           minY;
            int           maxY;
            int           yOffset;
            int           advance;
            unsigned long cached;
        };

        Font::Font(const std::shared_ptr<Utils::FreeType>& freetype, int pixelSize)
            : m_freetype(freetype)
            , m_face(nullptr)
            , m_valid(false)
            , m_current(nullptr)
            , m_pixelSize(pixelSize)
            , m_height(0)
            , m_fixedWidth(0)
            , m_ascent(0)
            , m_descent(0)
            , m_lineSkip(0)
            , m_fontSizeFamily(0)
            , m_faceStyle(0)
            , m_style(0)
            , m_outline(0)
            , m_kerning(0)
            , m_hinting(0)
            , m_glyphOverhang(0)
            , m_glyphItalics(0.0f)
            , m_underlineOffset(0)
            , m_underlineHeight(0)
        {
            m_cache = static_cast<struct Glyph*>(std::malloc(sizeof(struct Glyph) * CACHE_SIZE));
            if (m_cache != nullptr) {
                memset(m_cache, 0, sizeof(struct Glyph) * CACHE_SIZE);
            }
        }
        
        Font::~Font()
        {
            if (m_cache != nullptr) {
                FlushCache();
                std::free(m_cache);
            }
            
            
            if (m_face != nullptr) {
                FT_Done_Face(m_face);
            }
        }
        
        bool Font::Initialize(const std::string& path)
        {
            char*      fileBase;
            size_t     fileSize;
            FT_CharMap cmFound = 0;
            
            if (m_valid) {
                return true;
            }
            
            bool status = LoadFile(path, &fileBase, &fileSize);
            if (!status) {
                return status;
            }
            
            status = FT_New_Memory_Face(m_freetype->GetLibrary(),
                (const FT_Byte*)fileBase, fileSize, 0, &m_face) == FT_Err_Ok;
            if (!status) {
                std::free(fileBase);
                return status;
            }
        
            // Build the character map
            for (int i = 0; i < m_face->num_charmaps; i++) {
                FT_CharMap cm = m_face->charmaps[i];
                if ((cm->platform_id == 3 && cm->encoding_id == 1) ||   // Windows Unicode
                    (cm->platform_id == 3 && cm->encoding_id == 0) ||   // Windows Symbol
                    (cm->platform_id == 2 && cm->encoding_id == 1) ||   // ISO Unicode
                    (cm->platform_id == 0)) {                           // Apple Unicode
                    cmFound = cm;
                    break;
                }
            }
        
            // Keep using default character map in case we don't find it
            if (cmFound) {
                FT_Set_Charmap(m_face, cmFound);
            }
            
            // font is valid
            m_valid = true;
            return SetPixelSize(m_pixelSize);
        }
        
        bool Font::SetPixelSize(int pixelSize)
        {
            // Make sure that our font face is scalable (global metrics)
            if (FT_IS_SCALABLE(m_face)) {
                // Set the character size and use default DPI (72)
                if (FT_Set_Char_Size(m_face, 0, pixelSize * 64, 0, 0)) {
                    return false;
                }
        
                // Get the scalable font metrics for this font
                FT_Fixed scale      = m_face->size->metrics.y_scale;
                m_ascent            = FT_CEIL(FT_MulFix(m_face->ascender, scale));
                m_descent           = FT_CEIL(FT_MulFix(m_face->descender, scale));
                m_height            = m_ascent - m_descent + /* baseline */ 1;
                m_fixedWidth        = 0;
                m_lineSkip          = FT_CEIL(FT_MulFix(m_face->height, scale));
                m_underlineOffset   = FT_FLOOR(FT_MulFix(m_face->underline_position, scale));
                m_underlineHeight   = FT_FLOOR(FT_MulFix(m_face->underline_thickness, scale));
            }
            else {
                // Non-scalable font case.  ptsize determines which family
                // or series of fonts to grab from the non-scalable format.
                // It is not the point size of the font.
                if (pixelSize >= m_face->num_fixed_sizes) {
                    pixelSize = m_face->num_fixed_sizes - 1;
                }
        
                if (FT_Set_Pixel_Sizes(m_face, m_face->available_sizes[pixelSize].width, 
                        m_face->available_sizes[pixelSize].height)) {
                    return false;
                }
                m_fontSizeFamily = pixelSize;
        
                // With non-scalale fonts, Freetype2 likes to fill many of the
                // font metrics with the value of 0.  The size of the
                // non-scalable fonts must be determined differently
                // or sometimes cannot be determined.
                m_ascent            = m_face->available_sizes[pixelSize].height;
                m_fixedWidth        = m_face->available_sizes[pixelSize].width;
                m_descent           = 0;
                m_height            = m_face->available_sizes[pixelSize].height;
                m_lineSkip          = FT_CEIL(m_ascent);
                m_underlineOffset   = FT_FLOOR(m_face->underline_position);
                m_underlineHeight   = FT_FLOOR(m_face->underline_thickness);
            }
        
            if (m_underlineHeight < 1) {
                m_underlineHeight = 1;
            }
        
            // Initialize the font face style
            m_faceStyle = TTF_STYLE_NORMAL;
            if (m_face->style_flags & FT_STYLE_FLAG_BOLD) {
                m_faceStyle |= TTF_STYLE_BOLD;
            }
            if (m_face->style_flags & FT_STYLE_FLAG_ITALIC) {
                m_faceStyle |= TTF_STYLE_ITALIC;
            }
        
            // Update stored settings
            m_style         = m_faceStyle;
            m_outline       = 0;
            m_kerning       = 1;
            m_glyphOverhang = m_face->size->metrics.y_ppem / 10;
        
            // x offset = cos(((90.0-12)/360)*2*M_PI), or 12 degree angle
            m_glyphItalics  = 0.207f;
            m_glyphItalics *= m_face->height;

            // Guess the font width by getting 'M' as character
            if (!m_fixedWidth) {
                struct CharInfo bitmap;
                if (GetCharacterBitmap('M', bitmap)) {
                    m_fixedWidth = bitmap.width;
                }
            }
            return true;
        }
        
        bool Font::GetCharacterBitmap(unsigned long character, struct CharInfo& bitmap)
        {
            FT_Long       useKerning = FT_HAS_KERNING(m_face) && m_kerning;
            int           indentX    = 0;
            FT_Bitmap*    current;
            struct Glyph* glyph;
            FT_Error      status;
            int           width;
            
            if (!m_valid) {
                return false;
            }
        
            if (!FindGlyph(character, CACHED_METRICS | CACHED_PIXMAP)) {
                return false;
            }
            
            if (!m_current) {
                return false;
            }

            glyph   = m_current;
            current = &glyph->pixmap; // Use Bitmap if CACHED_BITMAP is set
        
            // Ensure the width of the pixmap is correct. On some cases,
            // freetype may report a larger pixmap than possible.
            width = current->width;
            if (m_outline <= 0 && width > glyph->maxX - glyph->minX) {
                width = glyph->maxX - glyph->minX;
            }
        
            // do kerning, if possible AC-Patch
            if (useKerning && bitmap.index && glyph->index) {
                FT_Vector delta;
                status = FT_Get_Kerning(m_face, bitmap.index, glyph->index, FT_KERNING_DEFAULT, &delta);
                if (!status) {
                    indentX += delta.x >> 6;
                }
            }
            indentX += glyph->minX;
            
            /* Handle the underline style
            if (TTF_HANDLE_STYLE_UNDERLINE(this)) {
                row = TTF_underline_top_row(font);
                TTF_drawLine_Solid(font, textbuf, row);
            } */
        
            /* Handle the strikethrough style
            if (TTF_HANDLE_STYLE_STRIKETHROUGH(this)) {
                row = TTF_strikethrough_top_row(font);
                TTF_drawLine_Solid(font, textbuf, row);
            }  */
        
            // Update the members
            bitmap.bitmap  = current->buffer;
            bitmap.pitch   = current->pitch;
            bitmap.width   = width;
            bitmap.height  = current->rows;
            bitmap.indentX = indentX;
            bitmap.indentY = glyph->yOffset;
            bitmap.advance = glyph->advance;
            bitmap.index   = glyph->index;
            if (TTF_HANDLE_STYLE_BOLD(this)) {
                bitmap.advance += m_glyphOverhang;
            }
            return true;
        }
        
        Rectangle Font::GetTextMetrics(const std::string& text)
        {
            struct CharInfo bitmapInfo = { 0 };
            int             width      = 0;
            int             height     = 0;
            for (size_t i = 0; i < text.length(); i++) {
                char character = text[i];
                
                if (GetCharacterBitmap(character, bitmapInfo)) {
                    width += bitmapInfo.indentX + bitmapInfo.advance;
                    if ((bitmapInfo.height + bitmapInfo.indentY) > height) {
                        height = bitmapInfo.height + bitmapInfo.indentY;
                    }
                }
            }
            return Rectangle(0, 0, width, height);
        }
        
        bool Font::LoadGlyph(unsigned long character, struct Glyph* cached, int want)
        {
            FT_Error          status = 0;
            FT_GlyphSlot      glyph;
            FT_Glyph_Metrics* metrics;
            FT_Outline*       outline;
        
            if (!m_face) {
                return false;
            }
            
            // Look up the character index, we will need it later
            if (!cached->index) {
                cached->index = FT_Get_Char_Index(m_face, character);
            }
        
            status = FT_Load_Glyph(m_face, cached->index, FT_LOAD_DEFAULT | m_hinting);
            if (status) {
                return false;
            }
        
            glyph   = m_face->glyph;
            metrics = &glyph->metrics;
            outline = &glyph->outline;
        
            // Get the glyph metrics if desired
            if ((want & CACHED_METRICS) && !(cached->stored & CACHED_METRICS)) {
                if (FT_IS_SCALABLE(m_face)) {
                    // Get the bounding box
                    cached->minX    = FT_FLOOR(metrics->horiBearingX);
                    cached->maxX    = FT_CEIL(metrics->horiBearingX + metrics->width);
                    cached->maxY    = FT_FLOOR(metrics->horiBearingY);
                    cached->minY    = cached->maxY - FT_CEIL(metrics->height);
                    cached->yOffset = m_ascent - cached->maxY;
                    cached->advance = FT_CEIL(metrics->horiAdvance);
                }
                else {
                    // Get the bounding box for non-scalable format.
                    // Again, freetype2 fills in many of the font metrics
                    // with the value of 0, so some of the values we
                    // need must be calculated differently with certain
                    // assumptions about non-scalable formats.
                    cached->minX    = FT_FLOOR(metrics->horiBearingX);
                    cached->maxX    = FT_CEIL(metrics->horiBearingX + metrics->width);
                    cached->maxY    = FT_FLOOR(metrics->horiBearingY);
                    cached->minY    = cached->maxY - FT_CEIL(m_face->available_sizes[m_fontSizeFamily].height);
                    cached->yOffset = 0;
                    cached->advance = FT_CEIL(metrics->horiAdvance);
                }
        
                // Adjust for bold and italic text
                if (TTF_HANDLE_STYLE_BOLD(this)) {
                    cached->maxX += m_glyphOverhang;
                }
                if (TTF_HANDLE_STYLE_ITALIC(this)) {
                    cached->maxX += (int)ceilf(m_glyphItalics);
                }
                cached->stored |= CACHED_METRICS;
            }
            
            // Do we have the glyph cached as bitmap/pixmap?
            if (((want & CACHED_BITMAP) && !(cached->stored & CACHED_BITMAP)) ||
                ((want & CACHED_PIXMAP) && !(cached->stored & CACHED_PIXMAP))) 
            {
                int        mono        = (want & CACHED_BITMAP);
                FT_Glyph   bitmapGlyph = nullptr;
                FT_Bitmap* source;
                FT_Bitmap* destination;
        
                // Handle the italic style
                if (TTF_HANDLE_STYLE_ITALIC(this))  {
                    FT_Matrix shear;
        
                    // Initialize shearing for glyph
                    shear.xx = 1 << 16;
                    shear.xy = (int)(m_glyphItalics * (1 << 16)) / m_height;
                    shear.yx = 0;
                    shear.yy = 1 << 16;
                    FT_Outline_Transform(outline, &shear);
                }
        
                // Handle outline rendering
                if ((m_outline > 0) && glyph->format != FT_GLYPH_FORMAT_BITMAP) {
                    FT_Stroker stroker;
                    
                    FT_Get_Glyph(glyph, &bitmapGlyph);
                    status = FT_Stroker_New(m_freetype->GetLibrary(), &stroker);
                    if (status) {
                        return false;
                    }
        
                    // Stroke the glyph, and clenaup the stroker
                    FT_Stroker_Set(stroker, m_outline * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
                    FT_Glyph_Stroke(&bitmapGlyph, stroker, 1 /* delete the original glyph */);
                    FT_Stroker_Done(stroker);
                    
                    // Render the glyph to a bitmap for easier re-rendering
                    status = FT_Glyph_To_Bitmap(&bitmapGlyph, mono ? FT_RENDER_MODE_MONO : FT_RENDER_MODE_NORMAL, 0, 1);
                    if (status) {
                        FT_Done_Glyph(bitmapGlyph);
                        return false;
                    }
                    
                    source = &((FT_BitmapGlyph)bitmapGlyph)->bitmap;
                }
                else {
                    status = FT_Render_Glyph(glyph, mono ? FT_RENDER_MODE_MONO : FT_RENDER_MODE_NORMAL);
                    if (status) {
                        return status;
                    }
                    
                    source = &glyph->bitmap;
                }
                
                // Copy over information to cache
                if (mono) {
                    destination = &cached->bitmap;
                }
                else {
                    destination = &cached->pixmap;
                }
                memcpy(destination, source, sizeof(*destination));
        
                // FT_Render_Glyph() and .fon fonts always generate a
                // two-color (black and white) glyphslot surface, even
                // when rendered in ft_render_mode_normal.
                // FT_IS_SCALABLE() means that the font is in outline format,
                // but does not imply that outline is rendered as 8-bit
                // grayscale, because embedded bitmap/graymap is preferred
                // (see FT_LOAD_DEFAULT section of FreeType2 API Reference).
                // FT_Render_Glyph() canreturn two-color bitmap or 4/16/256-
                // color graymap according to the format of embedded bitmap/
                // graymap.
                if (source->pixel_mode == FT_PIXEL_MODE_MONO) {
                    destination->pitch *= 8;
                }
                else if (source->pixel_mode == FT_PIXEL_MODE_GRAY2) {
                    destination->pitch *= 4;
                }
                else if (source->pixel_mode == FT_PIXEL_MODE_GRAY4) {
                    destination->pitch *= 2;
                }
        
                // Adjust for bold and italic text
                int bump = 0;
                if (TTF_HANDLE_STYLE_BOLD(this)) {
                    bump += m_glyphOverhang;
                }
                if (TTF_HANDLE_STYLE_ITALIC(this)) {
                    bump += (int)ceilf(m_glyphItalics);
                }
                destination->pitch += bump;
                destination->width += bump;
        
                if (destination->rows != 0) {
                    destination->buffer = (unsigned char*)malloc(destination->pitch * destination->rows);
                    if (!destination->buffer) {
                        return false;
                    }
                    memset(destination->buffer, 0, destination->pitch * destination->rows);
        
                    for (int i = 0; i < source->rows; i++) {
                        int soffset = i * source->pitch;
                        int doffset = i * destination->pitch;
                        if (mono) {
                            unsigned char *srcp = source->buffer + soffset;
                            unsigned char *dstp = destination->buffer + doffset;
                            int j;
                            if (source->pixel_mode == FT_PIXEL_MODE_MONO) {
                                for (j = 0; j < source->width; j += 8) {
                                    unsigned char c = *srcp++;
                                    *dstp++ = (c & 0x80) >> 7;
                                    c <<= 1;
                                    *dstp++ = (c & 0x80) >> 7;
                                    c <<= 1;
                                    *dstp++ = (c & 0x80) >> 7;
                                    c <<= 1;
                                    *dstp++ = (c & 0x80) >> 7;
                                    c <<= 1;
                                    *dstp++ = (c & 0x80) >> 7;
                                    c <<= 1;
                                    *dstp++ = (c & 0x80) >> 7;
                                    c <<= 1;
                                    *dstp++ = (c & 0x80) >> 7;
                                    c <<= 1;
                                    *dstp++ = (c & 0x80) >> 7;
                                }
                            }
                            else if (source->pixel_mode == FT_PIXEL_MODE_GRAY2) {
                                for (j = 0; j < source->width; j += 4) {
                                    unsigned char c = *srcp++;
                                    *dstp++ = (((c & 0xA0) >> 6) >= 0x2) ? 1 : 0;
                                    c <<= 2;
                                    *dstp++ = (((c & 0xA0) >> 6) >= 0x2) ? 1 : 0;
                                    c <<= 2;
                                    *dstp++ = (((c & 0xA0) >> 6) >= 0x2) ? 1 : 0;
                                    c <<= 2;
                                    *dstp++ = (((c & 0xA0) >> 6) >= 0x2) ? 1 : 0;
                                }
                            }
                            else if (source->pixel_mode == FT_PIXEL_MODE_GRAY4) {
                                for (j = 0; j < source->width; j += 2) {
                                    unsigned char c = *srcp++;
                                    *dstp++ = (((c & 0xF0) >> 4) >= 0x8) ? 1 : 0;
                                    c <<= 4;
                                    *dstp++ = (((c & 0xF0) >> 4) >= 0x8) ? 1 : 0;
                                }
                            }
                            else {
                                for (j = 0; j < source->width; j++) {
                                    unsigned char c = *srcp++;
                                    *dstp++ = (c >= 0x80) ? 1 : 0;
                                }
                            }
                        }
                        else if (source->pixel_mode == FT_PIXEL_MODE_MONO) {
                            // This special case wouldn't be here if the FT_Render_Glyph()
                            // function wasn't buggy when it tried to render a .fon font with 256
                            // shades of gray.  Instead, it returns a black and white surface
                            // and we have to translate it back to a 256 gray shaded surface.
                            unsigned char *srcp = source->buffer + soffset;
                            unsigned char *dstp = destination->buffer + doffset;
                            unsigned char c;
                            int j, k;
                            for (j = 0; j < source->width; j += 8) {
                                c = *srcp++;
                                for (k = 0; k < 8; ++k) {
                                    if ((c & 0x80) >> 7) {
                                        *dstp++ = NUM_GRAYS - 1;
                                    }
                                    else {
                                        *dstp++ = 0x00;
                                    }
                                    c <<= 1;
                                }
                            }
                        }
                        else if (source->pixel_mode == FT_PIXEL_MODE_GRAY2) {
                            unsigned char *srcp = source->buffer + soffset;
                            unsigned char *dstp = destination->buffer + doffset;
                            unsigned char c;
                            int j, k;
                            for (j = 0; j < source->width; j += 4) {
                                c = *srcp++;
                                for (k = 0; k < 4; ++k) {
                                    if ((c & 0xA0) >> 6) {
                                        *dstp++ = NUM_GRAYS * ((c & 0xA0) >> 6) / 3 - 1;
                                    }
                                    else {
                                        *dstp++ = 0x00;
                                    }
                                    c <<= 2;
                                }
                            }
                        }
                        else if (source->pixel_mode == FT_PIXEL_MODE_GRAY4) {
                            unsigned char *srcp = source->buffer + soffset;
                            unsigned char *dstp = destination->buffer + doffset;
                            unsigned char c;
                            int j, k;
                            for (j = 0; j < source->width; j += 2) {
                                c = *srcp++;
                                for (k = 0; k < 2; ++k) {
                                    if ((c & 0xF0) >> 4) {
                                        *dstp++ = NUM_GRAYS * ((c & 0xF0) >> 4) / 15 - 1;
                                    }
                                    else {
                                        *dstp++ = 0x00;
                                    }
                                    c <<= 4;
                                }
                            }
                        }
                        else {
                            memcpy(destination->buffer + doffset, source->buffer + soffset, source->pitch);
                        }
                    }
                }
        
                // Handle the bold style
                if (TTF_HANDLE_STYLE_BOLD(this))  {
                    uint8_t* pixmap;
                    int      row;
                    int      col;
                    int      offset;
                    int      pixel;
        
                    // The pixmap is a little hard, we have to add and clamp
                    for (row = destination->rows - 1; row >= 0; --row) {
                        pixmap = (uint8_t*)destination->buffer + row * destination->pitch;
                        for (offset = 1; offset <= m_glyphOverhang; ++offset) {
                            for (col = destination->width - 1; col > 0; --col) {
                                if (mono) {
                                    pixmap[col] |= pixmap[col - 1];
                                }
                                else {
                                    pixel = (pixmap[col] + pixmap[col - 1]);
                                    if (pixel > NUM_GRAYS - 1) {
                                        pixel = NUM_GRAYS - 1;
                                    }
                                    pixmap[col] = (uint8_t)pixel;
                                }
                            }
                        }
                    }
                }
        
                // Mark that we rendered this format
                if (mono) {
                    cached->stored |= CACHED_BITMAP;
                }
                else {
                    cached->stored |= CACHED_PIXMAP;
                }
        
                // Free outlined glyph
                if (bitmapGlyph) {
                    FT_Done_Glyph(bitmapGlyph);
                }
            }
        
            cached->cached = character;
            return true;
        }

        
        bool Font::FindGlyph(unsigned long character, int want)
        {
            // Get the appropriate index
            m_current = &m_cache[character % CACHE_SIZE];
            if (m_current->cached != 0 && m_current->cached != character) {
                FlushGlyph(m_current);
            }
        
            // check if it should be stored
            if ((m_current->stored & want) != want) {
                return LoadGlyph(character, m_current, want);
            }
            return true;
        }
        
        void Font::FlushGlyph(struct Glyph* glyph)
        {
            glyph->stored = 0;
            glyph->index  = 0;
            glyph->cached = 0;
        
            if (glyph->bitmap.buffer) {
                free(glyph->bitmap.buffer);
                glyph->bitmap.buffer = 0;
            }
            if (glyph->pixmap.buffer) {
                free(glyph->pixmap.buffer);
                glyph->pixmap.buffer = 0;
            }
        }
        
        void Font::FlushCache()
        {
            for (int i = 0; i < CACHE_SIZE; ++i) {
                if (m_cache[i].cached != 0) {
                    FlushGlyph(&m_cache[i]);
                }
            }
        }
    }
}
