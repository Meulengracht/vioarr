/* ValiOS
 *
 * Copyright 2018, Philip Meulengracht
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
#include "../utils/publisher.hpp"
#include <cstdint>
#include <map>
#include <memory>

namespace Asgaard {
    namespace Theming {
        class Theme;

        class ThemeManager final : public Utils::Publisher {
        public:
            ThemeManager();
            ~ThemeManager();

        public:
            ASGAARD_API const std::shared_ptr<Theme>& GetTheme() const;

        private:
            std::shared_ptr<Theme> m_theme;
        };
        
        extern ASGAARD_API ThemeManager TM;
    }
}
