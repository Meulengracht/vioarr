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

#include "register.hpp"
#include <theming/theme_manager.hpp>
#include <theming/theme.hpp>

namespace Heimdall
{
    namespace Register
    {
        namespace {
            static std::vector<std::unique_ptr<Application>> g_applicationRegister;
        }

        Application::Application(const std::string& name, const std::string& path, const Asgaard::Drawing::Image& icon)
            : m_name(name)
            , m_path(path)
            , m_icon(icon)
        {

        }

        void Initialize()
        {
            const auto theme = Asgaard::Theming::TM.GetTheme();
            auto termImage = theme->GetImage(Asgaard::Theming::Theme::Elements::IMAGE_TERMINAL);
            
            g_applicationRegister.push_back(std::make_unique<Application>("Terminal", "alumni", termImage));
        }

        void Update()
        {
            // nothing to do here
        }

        const std::vector<std::unique_ptr<Application>>& GetApplications()
        {
            return g_applicationRegister;
        }
    }
}
