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

#include "config.hpp"
#include "object.hpp"
#include <map>
#include <memory>

namespace Asgaard {
    class Surface;
    
    namespace Widgets {
        class Cursor;
    }

    class Pointer : public Object {
    public:
        enum class Buttons : int {
            LEFT = 0,
            MIDDLE,
            RIGHT
            // 2 > is possible
        };

        enum class DefaultCursors : int {
            ARROW,
            ARROW_WE,
            ARROW_ALL,
            BEAM
        };

    public:
        Pointer(uint32_t id);
        ~Pointer();
        
        ASGAARD_API void SetSurface(const std::shared_ptr<Widgets::Cursor>&, int xOffset = 0, int yOffset = 0);
        ASGAARD_API void SetDefaultSurface(const DefaultCursors cursor = DefaultCursors::ARROW);

    private:
        void Notification(const Publisher*, const Asgaard::Notification&) override;

    private:
        std::map<int, std::shared_ptr<Widgets::Cursor>> m_defaultCursors;
        std::shared_ptr<Widgets::Cursor>                m_currentCursor;
    };
}
