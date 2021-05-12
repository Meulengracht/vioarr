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

#include "protocols/wm_core_protocol_server.h"
#include "engine/vioarr_objects.h"
#include "engine/vioarr_utils.h"

void wm_core_sync_callback(struct gracht_recv_message* message, struct wm_core_sync_args* input)
{
    vioarr_utils_trace("[wm_core_sync_callback] client %i", message->client);
    wm_core_event_sync_single(message->client, input->serial);
}

void wm_core_get_objects_callback(struct gracht_recv_message* message)
{
    vioarr_utils_trace("[wm_core_get_objects_callback] client %i", message->client);
    vioarr_objects_publish(message->client);
}
