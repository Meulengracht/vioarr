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

#include "protocols/ctt_input_protocol_client.h"
#include "engine/vioarr_input.h"
#include "engine/vioarr_utils.h"

void ctt_input_event_properties_callback(struct ctt_input_properties_event* event)
{
    TRACE("[ctt_input_event_properties_callback] %u", event->device_type);
    vioarr_input_register(event->device_id,
        event->device_type == input_type_mouse ? 
            VIOARR_INPUT_POINTER : VIOARR_INPUT_KEYBOARD);
}

void ctt_input_event_button_callback(struct ctt_input_button_event* event)
{
    TRACE("[ctt_input_event_button_callback] %u", event->key_code);
    vioarr_input_button_event(event->device_id, (uint32_t)event->key_code, (uint32_t)event->modifiers);
}

void ctt_input_event_cursor_callback(struct ctt_input_cursor_event* event)
{
    TRACE("[ctt_input_event_button_callback] %i, %i, %i",
        event->rel_x, event->rel_y, event->rel_z);
    vioarr_input_axis_event(event->device_id, event->rel_x, event->rel_y, event->rel_z);
}
 