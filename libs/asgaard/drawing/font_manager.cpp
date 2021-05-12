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

#include "../include/drawing/font_manager.hpp"
#include "../include/drawing/font.hpp"
#include "../include/utils/freetype.hpp"

namespace Asgaard {
    namespace Drawing {
        FontManager FM;
    
        FontManager::FontManager()
        {
            m_freetype = std::shared_ptr<Utils::FreeType>(new Utils::FreeType());
        }
        
        FontManager::~FontManager()
        {
            
        }
        
        std::shared_ptr<Font> FontManager::CreateFont(const std::string& path, int pixelSize)
        {
            auto font = std::shared_ptr<Font>(new Font(m_freetype, pixelSize));
            if (!font->Initialize(path)) {
                return std::shared_ptr<Font>(nullptr);
            }
            return font;
        }
    }
}