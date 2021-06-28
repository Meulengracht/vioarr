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

namespace Heimdall
{
    namespace Register
    {
        namespace {
            static std::map<unsigned int, std::shared_ptr<Application>> g_applicationRegister;
        }

        Application::Application(const std::string&, const std::string&, const Asgaard::Drawing::Image&)
        {

        }

        void Initialize()
        {
            g_applicationRegister.push_back(std::make_pair(0, new Asgaard::Drawing::Color(0xFF, 0, 0, 0)))
        }

        void Update()
        {

        }

        std::shared_ptr<Application> GetApplication(unsigned int id)
        {
            auto app = g_applicationRegister.find(id);
            if (app == std::end(g_applicationRegister)) {
                return std::shared_ptr<Application>(nullptr);
            }
            return *app;
        }
    }
}
