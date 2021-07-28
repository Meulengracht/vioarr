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

#include <asgaard/drawing/image.hpp>
#include <vector>
#include <string>
#include <memory>

namespace Heimdall
{
    namespace Register
    {
        class Application
        {
        public:
            Application(const std::string&, const std::string&, const Asgaard::Drawing::Image&);
            ~Application() = default;

            const std::string&             GetName() const { return m_name; }
            const std::string&             GetPath() const { return m_path; }
            const Asgaard::Drawing::Image& GetIcon() const { return m_icon; }

        private:
            std::string             m_name;
            std::string             m_path;
            Asgaard::Drawing::Image m_icon;
        };

        void Initialize();
        void Update();

        const std::vector<std::unique_ptr<Application>>& GetApplications();
    }
}
