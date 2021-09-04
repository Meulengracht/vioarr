/* MollenOS
 *
 * Copyright 2020, Philip Meulengracht
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
 * Vioarr - Vali Compositor
 * - Implements the default system compositor for Vali. It utilizies the gracht library
 *   for communication between compositor clients and the server. The server renders
 *   using Mesa3D with either the soft-renderer or llvmpipe render for improved performance.
 */

#include "ctt_input_service_client.h"
#include "engine/vioarr_input.h"
#include "engine/vioarr_utils.h"
#include <os/keycodes.h>

void ctt_input_event_stats_invocation(gracht_client_t* client, const UUId_t deviceId, const enum ctt_input_type type)
{
    vioarr_utils_trace(VISTR("[ctt_input_event_properties_callback] %u"), type);
    vioarr_input_register(deviceId,
        type == CTT_INPUT_TYPE_MOUSE ? 
            VIOARR_INPUT_POINTER : VIOARR_INPUT_KEYBOARD);
}

void ctt_input_event_button_event_invocation(gracht_client_t* client, const UUId_t deviceId,
    const uint8_t keyCode, const uint16_t modifiers)
{
    uint8_t pressed = (modifiers & VK_MODIFIER_RELEASED) ? 0 : 1;
    vioarr_utils_trace(VISTR("[ctt_input_event_button_callback] %u"), keyCode);
    vioarr_input_button_event(deviceId, (uint32_t)keyCode, (uint32_t)modifiers, pressed);
}

void ctt_input_event_cursor_event_invocation(gracht_client_t* client, const UUId_t deviceId, 
    const uint16_t flags, const int16_t relX, const int16_t relY, const int16_t relZ)
{
    vioarr_utils_trace(VISTR("[ctt_input_event_button_callback] %i, %i, %i"),
        relX, relY, relZ);
    vioarr_input_axis_event(deviceId, relX, relY);
}
 