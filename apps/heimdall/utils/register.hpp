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
 * ValiOS - Application Environment (Launcher)
 *  - Contains the implementation of the application environment to support
 *    graphical user interactions.
 */
#pragma once

#include <map>
#include <string>
#include <drawing/image.hpp>

namespace Heimdall
{
    namespace Register
    {
        class Application
        {
        public:
            Application();
            ~Application();

            const std::string&             GetName() const;
            const std::string&             GetPath() const;
            const Asgaard::Drawing::Image& GetIcon() const;

        private:
            std::string             m_name;
            std::string             m_path;
            Asgaard::Drawing::Image m_icon;
        };

        void Initialize();
        void Update();
        const Application& GetApplication(unsigned int id);
    }
}
