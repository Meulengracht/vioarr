/**
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

#include "include/application.hpp"
#include "include/environment.hpp"
#include "include/memory_pool.hpp"
#include "include/memory_buffer.hpp"

#include "hd_core_service_client.h"

#include <functional>
#include <string>

namespace Asgaard {
namespace Environment {
namespace Heimdall {
// start of namespace

static bool                          g_isInitialized = false;
static std::string                   g_defaultGuid = "00000000-0000-0000-0000-000000000000";
static std::shared_ptr<MemoryPool>   g_iconPool;
static std::shared_ptr<MemoryBuffer> g_iconBuffer;

void RegisterApplication()
{
    if (!g_isInitialized) {
        return;
    }
}

void UnregisterApplication()
{
    if (!g_isInitialized) {
        return;
    }
}

void Initialize()
{
    if (g_isInitialized) {
        return;
    }

    // connect to heimdall

    // load GUID and hash
    auto guidPointer = APP.GetSettingValue<std::string*>(Application::Settings::APPLICATION_GUID);
    auto guid = g_defaultGuid;
    if (guidPointer) {
        guid = *guidPointer;
    }

    auto applicationId = std::hash<std::string>{}(guid);

    // load ICON
    auto iconPathPointer = APP.GetSettingValue<std::string*>(Application::Settings::APPLICATION_ICON);
    if (iconPathPointer)
    {
        auto iconPath = *iconPathPointer;
        Drawing::Image icon(iconPath);
        
        g_iconPool = OM.CreateClientObject<MemoryPool, std::size_t>(0);
        g_iconBuffer = OM.CreateClientObject<MemoryBuffer, 
            const std::shared_ptr<MemoryPool>&, int, int, int, enum PixelFormat>(
                g_iconPool, 0, width, height, PixelFormat::A8B8G8R8, MemoryBuffer::Flags::NONE
            );
    }

    // register application
    hd_core_register_app(nullptr, nullptr, applicationId, nullptr);

    g_isInitialized = true;
}

// end of namespace
} } }
