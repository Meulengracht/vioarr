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

#include <algorithm>
#include <asgaard/application.hpp>
#include <asgaard/object_manager.hpp>
#include <asgaard/memory_buffer.hpp>
#include <asgaard/surface.hpp>
#include <asgaard/screen.hpp>

#include <asgaard/events/surface_resize_event.hpp>
#include <asgaard/events/surface_focus_event.hpp>
#include <asgaard/events/pointer_enter_event.hpp>
#include <asgaard/events/pointer_leave_event.hpp>
#include <asgaard/events/pointer_move_event.hpp>
#include <asgaard/events/pointer_scroll_event.hpp>
#include <asgaard/events/pointer_click_event.hpp>
#include <asgaard/events/key_event.hpp>

#include <asgaard/notifications/focus_notification.hpp>

#include "wm_core_service_client.h"
#include "wm_screen_service_client.h"
#include "wm_memory_service_client.h"
#include "wm_surface_service_client.h"
#include "wm_pointer_service_client.h"

static enum Asgaard::Surface::SurfaceEdges GetSurfaceEdges(enum wm_surface_edge edges)
{
    // todo
    return Asgaard::Surface::SurfaceEdges::NONE;
}

namespace Asgaard {
    Surface::Surface(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions)
        : Object(id)
        , m_dimensions(dimensions)
        , m_screen(nullptr)
        , m_isFocused(false)
    {
        BindToScreen(screen);
    }
    
    Surface::Surface(uint32_t id, const Rectangle& dimensions)
        : Surface(id, std::shared_ptr<Screen>(nullptr), dimensions) { }

    Surface::~Surface()
    {
        wm_surface_destroy(APP.VioarrClient(), nullptr, Id());
    }
    
    void Surface::BindToScreen(const std::shared_ptr<Screen>& screen)
    {
        // If we previously were not attached to a screen and now are attaching
        // then we need to provide an underlying surface
        if (m_screen == nullptr && screen != nullptr) {
            // we do not allow surfaces to go beyond the screen size.
            if (m_dimensions.Width() > screen->GetCurrentWidth()) {
                m_dimensions.SetWidth(screen->GetCurrentWidth());
            }
            if (m_dimensions.Height() > screen->GetCurrentHeight()) {
                m_dimensions.SetHeight(screen->GetCurrentHeight());
            }

            wm_screen_create_surface(APP.VioarrClient(), nullptr, screen->Id(), Id(),    
                m_dimensions.X(), m_dimensions.Y(), m_dimensions.Width(), m_dimensions.Height());
        }
        m_screen = screen;
    }

    void Surface::AddChild(const std::shared_ptr<Surface>& child)
    {
        if (!child) {
            // error
            return;
        }

        // request subsurface
        wm_surface_add_subsurface(APP.VioarrClient(), nullptr, 
            Id(),
            child->Id(),
            child->Dimensions().X(),
            child->Dimensions().Y()
        );

        // subscribe to child
        child->Subscribe(this);
        m_children.push_back(child);
    }

    void Surface::ExternalEvent(const Event& event)
    {
        switch (event.GetType()) {
            case Event::Type::SURFACE_RESIZE: {
                const auto& resize = static_cast<const SurfaceResizeEvent&>(event);
                
                // When we get a resize event, the event is sent only to the parent surface
                // which equals this instance. Now we have to invoke the RESIZE event for all
                // registered children
                OnResized(GetSurfaceEdges(resize.Edges()), resize.Width(), resize.Height());
                m_dimensions.SetWidth(resize.Width());
                m_dimensions.SetHeight(resize.Height());
            } break;

            case Event::Type::SURFACE_FRAME: {
                OnFrame();
            } break;

            case Event::Type::KEY_EVENT: {
                const auto& key = static_cast<const KeyEvent&>(event);
                OnKeyEvent(key);
            } break;

            case Event::Type::SURFACE_FOCUSED: {
                const auto& focus = static_cast<const SurfaceFocusEvent&>(event);
                m_isFocused = focus.Focus();
                Notify(FocusEventNotification(Id(), m_isFocused));
                OnFocus(focus.Focus());
            } break;

            case Event::Type::POINTER_ENTER: {
                const auto& enter = static_cast<const PointerEnterEvent&>(event);
                auto pointer = Asgaard::OM[enter.PointerId()];
                OnMouseEnter(std::dynamic_pointer_cast<Pointer>(pointer), enter.LocalX(), enter.LocalY());
            } break;

            case Event::Type::POINTER_LEAVE: {
                const auto& leave = static_cast<const PointerLeaveEvent&>(event);
                auto pointer = Asgaard::OM[leave.PointerId()];
                OnMouseLeave(std::dynamic_pointer_cast<Pointer>(pointer));
            } break;

            case Event::Type::POINTER_MOVE: {
                const auto& move = static_cast<const PointerMoveEvent&>(event);
                auto pointer = Asgaard::OM[move.PointerId()];
                OnMouseMove(std::dynamic_pointer_cast<Pointer>(pointer), move.LocalX(), move.LocalY());
            } break;

            case Event::Type::POINTER_SCROLL: {
                const auto& scroll = static_cast<const PointerScrollEvent&>(event);
                auto pointer = Asgaard::OM[scroll.PointerId()];
                OnMouseScroll(std::dynamic_pointer_cast<Pointer>(pointer), scroll.ChangeX(), scroll.ChangeY());
            } break;

            case Event::Type::POINTER_CLICK: {
                const auto& click = static_cast<const PointerClickEvent&>(event);
                auto pointer = Asgaard::OM[click.PointerId()];
                OnMouseClick(
                    std::dynamic_pointer_cast<Pointer>(pointer), 
                    static_cast<enum Pointer::Buttons>(click.Button()),
                    click.Pressed());
            } break;

            default:
                break;
        }
        
        // always call base-handler for these types
        Object::ExternalEvent(event);
    }

    void Surface::Notification(const Publisher* source, const Asgaard::Notification& notification)
    {
        /**
         * We must listen to destroy events from our subscriptions, because if one of our
         * children wants to destroy, then we must release the pointer we have for that child
         */
        if (notification.GetType() == NotificationType::DESTROY) {
            auto child = std::find_if(std::begin(m_children), std::end(m_children),
                [id = notification.GetObjectId()] (const std::shared_ptr<Surface>& i) {
                    return i->Id() == id;
            });
            if (child != std::end(m_children)) {
                m_children.erase(child);
            }
        }

        /**
         * Focus events are only ever sent by our self, and the focus is already handled.
         * So when we receive a FOCUS_EVENT we must propegate the event up, and to our other
         * children.
         */
        else if (notification.GetType() == NotificationType::FOCUS_EVENT) {
            /**
             * If the event came from one of our children, then we must update the rest
             * of our children, and then send the event further up the chain
             */
            auto childEntry = std::find_if(std::begin(m_children), std::end(m_children),
                [&notification](const std::shared_ptr<Surface>& i) { return i->Id() == notification.GetObjectId(); });
            if (childEntry != std::end(m_children)) {
                m_isFocused = (*childEntry)->IsFocused();
                // clear focus for all other children
                std::for_each(std::begin(m_children), std::end(m_children),
                [this, &notification](const std::shared_ptr<Surface>& i) {
                    if (notification.GetObjectId() != i->Id()) {
                        i->Notification(this, FocusNotification(Id(), false));
                    }
                });
            }

            // snitch to our parents
            Notify(FocusEventNotification(Id(), m_isFocused));
        }
        else if (notification.GetType() == NotificationType::FOCUS) {
            /**
             * This events comes as an internal event, and we must update our
             * focus and invoke OnFocus. No other actions should be taken here 
             */
            const auto& focusNotification = static_cast<const FocusNotification&>(notification);
            m_isFocused = focusNotification.Focus();
            OnFocus(m_isFocused);
        }
        
        Object::Notification(source, notification);
    }
    
    void Surface::SetBuffer(const std::shared_ptr<MemoryBuffer>& buffer)
    {
        uint32_t id = 0;
        if (buffer) { 
            id = buffer->Id();
        }

        wm_surface_set_buffer(APP.VioarrClient(), nullptr, Id(), id);
    }
    
    void Surface::MarkDamaged(const Rectangle& dimensions)
    {
        wm_surface_invalidate(APP.VioarrClient(), nullptr, Id(),
            dimensions.X(), dimensions.Y(),
            dimensions.Width(), dimensions.Height());
    }
    
    void Surface::MarkInputRegion(const Rectangle& dimensions)
    {
        wm_surface_set_input_region(APP.VioarrClient(), nullptr, Id(),
            dimensions.X(), dimensions.Y(),
            dimensions.Width(), dimensions.Height());
    }

    void Surface::SetDropShadow(const Rectangle& dimensions)
    {
        wm_surface_set_drop_shadow(APP.VioarrClient(), nullptr, Id(),
            dimensions.X(), dimensions.Y(), dimensions.Width(), dimensions.Height());
    }

    void Surface::RequestPriorityLevel(enum PriorityLevel level)
    {
        wm_surface_request_level(APP.VioarrClient(), nullptr, Id(),
            static_cast<int>(level));
    }

    void Surface::RequestFullscreenMode(enum FullscreenMode mode)
    {
        wm_surface_request_fullscreen_mode(APP.VioarrClient(), nullptr, Id(), 
            static_cast<wm_fullscreen_mode>(mode));
    }

    void Surface::RequestFocus()
    {
        wm_surface_request_focus(APP.VioarrClient(), nullptr, Id());
    }
    
    void Surface::TransferFocus(uint32_t globalId)
    {
        wm_surface_request_focus(APP.VioarrClient(), nullptr, globalId);
    }
    
    void Surface::ApplyChanges()
    {
        wm_surface_commit(APP.VioarrClient(), nullptr, Id());
    }
    
    void Surface::RequestFrame()
    {
        wm_surface_request_frame(APP.VioarrClient(), nullptr, Id());
    }

    void Surface::GrabPointer(const std::shared_ptr<Pointer>& pointer)
    {
        wm_pointer_grab(APP.VioarrClient(), nullptr, pointer->Id(), Id());
    }

    void Surface::UngrabPointer(const std::shared_ptr<Pointer>& pointer)
    {
        wm_pointer_ungrab(APP.VioarrClient(), nullptr, pointer->Id(), Id());
    }
}
