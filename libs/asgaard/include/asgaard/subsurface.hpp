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
#include "surface.hpp"

namespace Asgaard {
    class Screen;
    
    class SubSurface : public Surface {
    public:
        ASGAARD_API SubSurface(uint32_t, const std::shared_ptr<Screen>&, const Rectangle&);

    public:
        template<class S, typename... Params>
        static std::shared_ptr<S> Create(Surface* parent, Params... parameters) {
            if (!std::is_base_of<SubSurface, S>::value) {
                return nullptr;
            }

            auto subsurface = OM.CreateClientObject<S, const std::shared_ptr<Screen>&, Params...>(
                parent->GetScreen(), std::forward<Params>(parameters)...);
            if (subsurface == nullptr) {
                return nullptr;
            }

            parent->AddChild(subsurface);
            return subsurface;
        }
    
    public:
        ASGAARD_API void Resize(int width, int height);
        ASGAARD_API void Move(int parentX, int parentY);
    };
}
