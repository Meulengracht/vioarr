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

#include "include/application.hpp"
#include "include/window_base.hpp"
#include "include/window_decoration.hpp"
#include "include/window_edge.hpp"
#include "include/object_manager.hpp"
#include "include/memory_pool.hpp"
#include "include/memory_buffer.hpp"
#include "include/pointer.hpp"

#include "wm_core_protocol_client.h"
#include "wm_screen_protocol_client.h"
#include "wm_memory_protocol_client.h"
#include "wm_surface_protocol_client.h"

static enum Asgaard::Surface::SurfaceEdges GetWindowEdges(enum wm_surface_edge edges)
{
    return Asgaard::Surface::SurfaceEdges::NONE;
}

static enum wm_surface_edge ToWindowEdges(enum Asgaard::Surface::SurfaceEdges edges)
{
    return no_edges;
}

namespace Asgaard {
    WindowBase::WindowBase(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions)
        : Surface(id, screen, dimensions)
    {
        Rectangle decorationDimensions(0, 0, dimensions.Width(), 35);
        m_decoration = OM.CreateClientObject<Asgaard::WindowDecoration>(screen, this, decorationDimensions);
        m_decoration->Subscribe(this);

        // install in right lower corner
        Rectangle edgeDimensions(dimensions.Width() - 16, dimensions.Height() - 16, 16, 16);
        m_edge = OM.CreateClientObject<Asgaard::WindowEdge>(screen, this, edgeDimensions);
        m_edge->Subscribe(this);

        // retrieve a list of supported window content formats
        wm_surface_get_formats(APP.GrachtClient(), nullptr, Id());
        wm_core_sync(APP.GrachtClient(), nullptr, Id());
    }

    WindowBase::~WindowBase()
    {
        Destroy();   
    }

    void WindowBase::Destroy()
    {
        if (m_decoration) { m_decoration->Unsubscribe(this); }
        if (m_edge)       { m_edge->Unsubscribe(this); }

        // invoke base destroy
        Surface::Destroy();
    }

    void WindowBase::SetTitle(const std::string& title)
    {
        m_decoration->SetTitle(title);
    }

    void WindowBase::SetIconImage(const std::shared_ptr<Drawing::Image>& image)
    {
        m_decoration->SetImage(image);
    }

    void WindowBase::SetResizable(bool resizeable)
    {
        m_edge->SetVisible(resizeable);
    }

    void WindowBase::EnableDecoration(bool enable)
    {
        m_decoration->SetVisible(enable);
    }

    void WindowBase::RequestPriorityLevel(enum PriorityLevel level)
    {
        wm_surface_request_level(APP.GrachtClient(), nullptr, Id(),
            static_cast<int>(level));
    }

    void WindowBase::RequestFullscreenMode(enum FullscreenMode mode)
    {
        wm_surface_request_fullscreen_mode(APP.GrachtClient(), nullptr, Id(), 
            static_cast<wm_surface_fullscreen_mode>(mode));
    }
    
    void WindowBase::InitiateResize(const std::shared_ptr<Pointer>& pointer, enum SurfaceEdges edges)
    {
        wm_surface_resize(APP.GrachtClient(), nullptr, Id(), pointer->Id(), ToWindowEdges(edges));
    }
    
    void WindowBase::InitiateMove(const std::shared_ptr<Pointer>& pointer)
    {
        wm_surface_move(APP.GrachtClient(), nullptr, Id(), pointer->Id());
    }
    
    void WindowBase::Notification(Publisher* source, int event, void* data)
    {
        auto object = dynamic_cast<Object*>(source);
        if (object) {
            if (object->Id() == m_decoration->Id()) {
                switch (event) {
                    case static_cast<int>(WindowDecoration::Notification::MINIMIZE): {
                        OnMinimize();

                        auto nullPointer = std::shared_ptr<MemoryBuffer>(nullptr);
                        SetBuffer(nullPointer);
                    } break;
                    case static_cast<int>(WindowDecoration::Notification::MAXIMIZE): {
                        OnMaximize();
                        RequestFullscreenMode(FullscreenMode::NORMAL);
                    } break;
                    case static_cast<int>(WindowDecoration::Notification::INITIATE_DRAG): {
                        auto pointerId = static_cast<uint32_t>(reinterpret_cast<intptr_t>(data));
                        auto pointer = std::dynamic_pointer_cast<Asgaard::Pointer>(Asgaard::OM[pointerId]);
                        InitiateMove(pointer);
                    } break;

                    default: break;
                }
            }
            else if (object->Id() == m_edge->Id()) {
                if (event == static_cast<int>(WindowEdge::Notification::INITIATE_DRAG)) {
                    auto pointerId = static_cast<uint32_t>(reinterpret_cast<intptr_t>(data));
                    auto pointer = std::dynamic_pointer_cast<Asgaard::Pointer>(Asgaard::OM[pointerId]);
                    InitiateResize(pointer, Surface::SurfaceEdges::RIGHT | Surface::SurfaceEdges::BOTTOM);
                }
            }
            else {
                switch (event) {
                    case static_cast<int>(MemoryBuffer::Notification::REFRESHED): {
                        OnRefreshed(dynamic_cast<MemoryBuffer*>(object));
                    } break;
                }
            }
        }
    }
    
    void WindowBase::ExternalEvent(enum ObjectEvent event, void* data)
    {
        switch (event)
        {
            case ObjectEvent::SURFACE_FORMAT: {
                struct wm_surface_format_event* event = 
                    (struct wm_surface_format_event*)data;
                m_supportedFormats.push_back((enum PixelFormat)event->format);
            } break;
            
            case ObjectEvent::SYNC: {
                OnCreated();
            } break;
            
            default:
                break;
        }
        
        // Run the base class events as well
        Surface::ExternalEvent(event, data);
    }
}
