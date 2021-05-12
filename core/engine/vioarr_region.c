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
 
#include "vioarr_region.h"
#include "vioarr_utils.h"
#include <stdlib.h>

typedef struct vioarr_region {
    int x;
    int y;
    int width;
    int height;
} vioarr_region_t;

vioarr_region_t* vioarr_region_create(void)
{
    vioarr_region_t* region;
    
    region = malloc(sizeof(vioarr_region_t));
    if (!region) {
        return NULL;
    }
    
    vioarr_region_zero(region);
    return region;
}

void vioarr_region_zero(vioarr_region_t* region)
{
    if (!region) {
        return;
    }
    
    region->x = 0;
    region->y = 0;
    region->width = 0;
    region->height = 0;
}

void vioarr_region_copy(vioarr_region_t* target, vioarr_region_t* source)
{
    if (!target || !source) {
        return;
    }

    target->x = source->x;
    target->y = source->y;
    target->width = source->width;
    target->height = source->height;
}

void vioarr_region_set_position(vioarr_region_t* region, int x, int y)
{
    if (!region) {
        return;
    }

    region->x = x;
    region->y = y;
}

void vioarr_region_set_size(vioarr_region_t* region, int width, int height)
{
    if (!region) {
        return;
    }

    region->width  = width;
    region->height = height;
}

void vioarr_region_add(vioarr_region_t* region, int x, int y, int width, int height)
{
    if (!region) {
        return;
    }
    
    if (x < region->x) {
        region->x = x;
    }
    
    if (y < region->y) {
        region->y = y;
    }
    
    if (width > region->width) {
        region->width = width;
    }
    
    if (height > region->height) {
        region->height = height;
    }
}

int vioarr_region_x(vioarr_region_t* region)
{
    if (!region) {
        return 0;
    }
    return region->x;
}

int vioarr_region_y(vioarr_region_t* region)
{
    if (!region) {
        return 0;
    }
    return region->y;
}

int vioarr_region_width(vioarr_region_t* region)
{
    if (!region) {
        return 0;
    }
    return region->width;
}

int vioarr_region_height(vioarr_region_t* region)
{
    if (!region) {
        return 0;
    }
    return region->height;
}


int vioarr_region_is_zero(vioarr_region_t* region)
{
    if (!region) {
        return 0;
    }
    
    if (region->x == 0 && region->y == 0 &&
            region->width == 0 && region->height == 0) {
        return 1;            
    }
    return 0;
}

int vioarr_region_contains(vioarr_region_t* region, int x , int y)
{
    if (!region) {
        return 0;
    }

    if (region->width == 0 || region->height == 0) {
        return 0;
    }

    return
        x >= region->x && y >= region->y &&
        x < (region->x + region->width) && 
        y < (region->y + region->height);
}

int vioarr_region_intersects(vioarr_region_t* region1, vioarr_region_t* region2)
{
    if (!region1 || !region2) {
        return 0;
    }

    if ( region1->x                    < (region2->x + region2->width)  &&
        (region1->x + region1->width)  >  region2->x                    &&
         region1->y                    < (region2->y + region2->height) &&
        (region1->y + region1->height) >  region2->y) {
        return 1;
    }
    return 0;
}
