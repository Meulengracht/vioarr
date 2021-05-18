/* ValiOS
 *
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
#pragma once

#include <string>
#include "event.hpp"

#include "wm_core_service.h" // for wm_surface_edge

namespace Asgaard {
    class SurfaceResizeEvent : public Event {
    public:
        SurfaceResizeEvent(const int width, const int height, const enum wm_surface_edge edges) 
        : Event(Event::Type::SURFACE_RESIZE)
        , m_width(width)
        , m_height(height)
        , m_edges(edges)
        { }

        int                  Width() const { return m_width; }
        int                  Height() const { return m_height; }
        enum wm_surface_edge Edges() const { return m_edges; }

    private:
        int                  m_width;
        int                  m_height;
        enum wm_surface_edge m_edges;
    };
}
