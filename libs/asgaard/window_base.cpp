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

#include "include/events/surface_format_event.hpp"
#include "include/notifications/draginitiated_notification.hpp"

#include "wm_core_service_client.h"
#include "wm_screen_service_client.h"
#include "wm_memory_service_client.h"
#include "wm_surface_service_client.h"

static enum Asgaard::Surface::SurfaceEdges GetWindowEdges(enum wm_surface_edge edges)
{
    return Asgaard::Surface::SurfaceEdges::NONE;
}

static enum wm_surface_edge ToWindowEdges(enum Asgaard::Surface::SurfaceEdges edges)
{
    return WM_SURFACE_EDGE_NO_EDGES;
}

namespace Asgaard {
    WindowBase::WindowBase(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions)
        : Surface(id, screen, dimensions)
    {
        Rectangle decorationDimensions(0, 0, dimensions.Width(), 35);
        m_decoration = SubSurface::Create<Asgaard::WindowDecoration>(this, decorationDimensions);

        // install in right lower corner
        Rectangle edgeDimensions(dimensions.Width() - 16, dimensions.Height() - 16, 16, 16);
        m_edge = SubSurface::Create<Asgaard::WindowEdge>(this, edgeDimensions);

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

    void WindowBase::InitiateResize(const std::shared_ptr<Pointer>& pointer, enum SurfaceEdges edges)
    {
        wm_surface_resize(APP.GrachtClient(), nullptr, Id(), pointer->Id(), ToWindowEdges(edges));
    }
    
    void WindowBase::InitiateMove(const std::shared_ptr<Pointer>& pointer)
    {
        wm_surface_move(APP.GrachtClient(), nullptr, Id(), pointer->Id());
    }
    
    void WindowBase::Notification(const Publisher* source, const Asgaard::Notification& notification)
    {
        switch (notification.GetType())
        {
            case NotificationType::DRAG_INITIATED: {
                const auto& dragNotification = reinterpret_cast<const DragInitiatedNotification&>(notification);
                if (notification.GetObjectId() == m_decoration->Id()) {
                    auto pointer = std::dynamic_pointer_cast<Asgaard::Pointer>(Asgaard::OM[dragNotification.GetPointerId()]);
                    InitiateMove(pointer);
                }
                else if (notification.GetObjectId() == m_edge->Id()) {
                    auto pointer = std::dynamic_pointer_cast<Asgaard::Pointer>(Asgaard::OM[dragNotification.GetPointerId()]);
                    InitiateResize(pointer, Surface::SurfaceEdges::RIGHT | Surface::SurfaceEdges::BOTTOM);
                }
            } break;

            case NotificationType::MINIMIZE: {
                OnMinimize();

                auto nullPointer = std::shared_ptr<MemoryBuffer>(nullptr);
                SetBuffer(nullPointer);
            } break;

            case NotificationType::MAXIMIZE: {
                OnMaximize();
                RequestFullscreenMode(FullscreenMode::NORMAL);
            } break;

            case NotificationType::REFRESHED: {
                OnRefreshed(dynamic_cast<const MemoryBuffer*>(source));
            } break;

            default:
                break;
        }

        Surface::Notification(source, notification);
    }
    
    void WindowBase::ExternalEvent(const Event& event)
    {
        switch (event.GetType())
        {
            case Event::Type::SURFACE_FORMAT: {
                const auto& format = static_cast<const SurfaceFormatEvent&>(event);
                m_supportedFormats.push_back((enum PixelFormat)format.Format());
            } break;
            
            case Event::Type::SYNC: {
                OnCreated();
            } break;
            
            default:
                break;
        }
        
        // Run the base class events as well
        Surface::ExternalEvent(event);
    }
}
