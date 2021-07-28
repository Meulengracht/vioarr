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

#include <asgaard/application.hpp>
#include <asgaard/memory_pool.hpp>
#include <asgaard/memory_buffer.hpp>
#include <asgaard/drawing/image.hpp>
#include <asgaard/exceptions/application_exception.h>

#include "hd_core_service_client.h"
#include "environment_private.hpp"

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

namespace {
    std::string getApplicationGuid()
    {
        auto providedGuid = APP.GetSettingString(Application::Settings::APPLICATION_GUID);
        if (providedGuid.size()) {
            return providedGuid;
        }
        return g_defaultGuid;
    }
}

void RegisterSurface(uint32_t globalId)
{
    if (!g_isInitialized) {
        return;
    }

    auto guid = getApplicationGuid();
    auto applicationId = std::hash<std::string>{}(guid);

    hd_core_register_surface(APP.HeimdallClient(), nullptr, applicationId, globalId);
}

void Initialize()
{
    if (g_isInitialized) {
        throw ApplicationException("Heimdall::Initialize was called a second time!", -1);
    }

    // connect to heimdall
    auto status = gracht_client_connect(APP.HeimdallClient());
    if (status) {
        throw ApplicationException("failed to connect to heimdall server", status);
    }

    // load GUID and hash
    auto guid = getApplicationGuid();
    auto applicationId = std::hash<std::string>{}(guid);

    // load ICON
    hd_app_icon iconDescriptor{};
    auto iconPath = APP.GetSettingString(Application::Settings::APPLICATION_ICON);
    if (iconPath.size())
    {
        Drawing::Image icon(iconPath);
        if (icon.Width() == 0) {
            // invalid path
            throw ApplicationException("Settings::APPLICATION_ICON was given an invalid path", -1);
        }
        
        auto size = icon.Stride() * icon.Height();
        g_iconPool = OM.CreateClientObject<MemoryPool, std::size_t>(size);
        g_iconBuffer = OM.CreateClientObject<MemoryBuffer, 
            const std::shared_ptr<MemoryPool>&, int, int, int, enum PixelFormat>(
                g_iconPool, 0, icon.Width(), icon.Height(), PixelFormat::A8B8G8R8, 
                MemoryBuffer::Flags::NONE
            );
        
        // configure the struct
        iconDescriptor.poolHandle = g_iconPool->Handle();
        iconDescriptor.size       = size;
        iconDescriptor.iconWidth  = icon.Width();
        iconDescriptor.iconHeight = icon.Height();
        iconDescriptor.format     = static_cast<int>(PixelFormat::A8B8G8R8);
    }

    // register application
    hd_core_register_app(APP.HeimdallClient(), nullptr, applicationId, &iconDescriptor);

    // setup done
    g_isInitialized = true;
}

void Destroy()
{
    if (!g_isInitialized) {
        return;
    }

    auto guid = getApplicationGuid();
    auto applicationId = std::hash<std::string>{}(guid);

    hd_core_unregister_app(APP.HeimdallClient(), nullptr, applicationId);
}

// end of namespace
} } }

extern "C"
{
    void hd_core_event_notification_input_invocation(gracht_client_t* client, const unsigned int id, const char* input)
    {

    }

    void hd_core_event_notification_response_invocation(gracht_client_t* client, const unsigned int id, const enum hd_buttons button)
    {

    }
}
