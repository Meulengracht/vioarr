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

#include "vioarr_buffer.h"
#include "vioarr_memory.h"
#include "vioarr_objects.h"
#include "vioarr_surface.h"
#include "vioarr_utils.h"
#include "../protocols/wm_core_protocol_server.h"
#include <ds/list.h>
#include <stdatomic.h>
#include <stdlib.h>
 
#define SERVER_ID_START 0x80000000

typedef struct vioarr_object {
    int                      client;
    uint32_t                 id;
    enum wm_core_object_type type;
    void*                    object;
    UUId_t                   handle;
    element_t                link;
} vioarr_object_t;
 
static _Atomic(uint32_t) object_id = ATOMIC_VAR_INIT(SERVER_ID_START);
static list_t            objects   = LIST_INIT;
 
static uint32_t vioarr_utils_get_object_id(void)
{
    return atomic_fetch_add(&object_id, 1);
}

void vioarr_objects_create_client_object(int client, uint32_t id, void* object, enum wm_core_object_type type)
{
    vioarr_object_t* resource;
    
    resource = malloc(sizeof(vioarr_object_t));
    if (!resource) {
        return;
    }
    
    resource->client = client;
    resource->id     = id;
    resource->object = object;
    resource->type   = type;
    ELEMENT_INIT(&resource->link, (uintptr_t)resource->id, resource);
    
    list_append(&objects, &resource->link);
}

uint32_t vioarr_objects_create_server_object(void* object, enum wm_core_object_type type)
{
    vioarr_object_t* resource;
    
    resource = malloc(sizeof(vioarr_object_t));
    if (!resource) {
        return 0;
    }
    
    resource->client = -1;
    resource->id     = vioarr_utils_get_object_id();
    resource->object = object;
    resource->type   = type;
    ELEMENT_INIT(&resource->link, (uintptr_t)resource->id, resource);
    
    list_append(&objects, &resource->link);
    return resource->id;
}

// Returns the object that matches the id - either if the id is a global id
// and matches or if the client and id matches
static vioarr_object_t* get_object(int client, uint32_t id)
{
    foreach(i, &objects) {
        vioarr_object_t* object = i->value;
        if ((id >= SERVER_ID_START && object->id == id) ||
            (object->client == client && object->id == id)) {
            return object;
        }
    }
    ERROR("[vioarr_objects_get_object] %i => %u object not found", client, id);
    return NULL;
}

// Returns the object that matches the id - either if the id is a global id
// and matches or if the client and id matches
void* vioarr_objects_get_object(int client, uint32_t id)
{
    vioarr_object_t* object = get_object(client, id);
    if (!object){
        return NULL;
    }
    return object->object;
}

int vioarr_objects_remove_object(int client, uint32_t id)
{
    vioarr_object_t* object = get_object(client, id);
    if (!object){
        return -1;
    }
    
    list_remove(&objects, &object->link);
    if (id >= SERVER_ID_START) {
        wm_core_event_object_all(id, object->handle, object->type);
    }
    free(object);
    return 0;
}

void vioarr_objects_remove_by_client(int client)
{
    #define CLEANUP_TYPE(object_type, dctor) \
        _foreach_nolink(i, &objects) { \
            vioarr_object_t* object = i->value; \
            if (object->client == client) { \
                element_t* next = i->next; \
                if (object->type == object_type) { \
                    dctor(object->object); \
                } \
                list_remove(&objects, i); \
                free(i->value); \
                i = next; \
            } \
            else { \
                i = i->next; \
            } \
        }


    // When we clean objects up due to disconnect, we want to go through
    // and make sure we up in this order:
    // surfaces
    // buffers
    // pools
    element_t* i;
    CLEANUP_TYPE(object_type_surface, vioarr_surface_destroy)
    CLEANUP_TYPE(object_type_buffer, vioarr_buffer_destroy)
    CLEANUP_TYPE(object_type_memory_pool, vioarr_memory_destroy_pool)
}

// publishes all server objects
void vioarr_objects_publish(int client)
{
    foreach(i, &objects) {
        vioarr_object_t* object = i->value;
        if (object->client == -1) {
            wm_core_event_object_single(client, object->id, object->handle, object->type);
        }
    }
}
