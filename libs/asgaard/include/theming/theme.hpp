/* ValiOS
 *
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

#include "../drawing/image.hpp"
#include "../drawing/color.hpp"
#include <map>
#include <memory>

namespace Asgaard {
    namespace Theming {
        class ThemeLoader;
        class Theme {
        public:
            enum class Elements : int {
                // Environment elements
                IMAGE_CURSOR,

                // Window elements
                IMAGE_CLOSE,
                IMAGE_MAXIMIZE,
                IMAGE_MINIMIZE,
                IMAGE_APP_DEFAULT,

                // UI Elements
                IMAGE_SEARCH,
                IMAGE_TERMINAL,
                IMAGE_EDITOR,
                IMAGE_GAME
            };

            enum class Colors : int {
                DECORATION_FILL,
                DECORATION_TEXT
            };
        public:
            Theme(const std::string& themePack);
            ~Theme();

            ASGAARD_API Drawing::Image GetImage(enum Elements element);
            ASGAARD_API Drawing::Color GetColor(enum Colors color);

        private:
            void InitializeTheme();

        private:
            std::unique_ptr<ThemeLoader>  m_loader;
            std::map<int, Drawing::Color> m_colors;
            std::map<int, std::string>    m_paths;
        };
    }
}
