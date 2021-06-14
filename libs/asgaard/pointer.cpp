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
#include "include/pointer.hpp"
#include "include/surface.hpp"
#include "include/theming/theme_manager.hpp"
#include "include/theming/theme.hpp"
#include "include/widgets/cursor.hpp"

#include "wm_pointer_service_client.h"

using namespace Asgaard;

constexpr auto CURSOR_SIZE = 16;

namespace {
    Theming::Theme::Elements ConvertToThemeImageIndex(const Pointer::DefaultCursors cursor)
    {
        switch (cursor) {
            case Pointer::DefaultCursors::ARROW:
                return Theming::Theme::Elements::IMAGE_CURSOR;

            case Pointer::DefaultCursors::ARROW_WE:
                return Theming::Theme::Elements::IMAGE_CURSOR_ARROW_WE;

            case Pointer::DefaultCursors::ARROW_ALL:
                return Theming::Theme::Elements::IMAGE_CURSOR_ARROW_ALL;

            case Pointer::DefaultCursors::BEAM:
                return Theming::Theme::Elements::IMAGE_CURSOR_BEAM;

            default:
                return Theming::Theme::Elements::IMAGE_CURSOR;
        }
    }
}

Pointer::Pointer(uint32_t id) 
    : Object(id)
{ }

Pointer::~Pointer()
{ }

void Pointer::SetSurface(const std::shared_ptr<Widgets::Cursor>& cursor, int xOffset, int yOffset)
{
    uint32_t id = 0;
    if (cursor) {
        id = cursor->Id();
    }

    // store pointer to unsubscribe
    if (m_currentCursor) {
        m_currentCursor->Hide();
        m_currentCursor->Unsubscribe(this);
        m_currentCursor.reset();
    }
    
    // calling with an id of 0 will result in clearing the pointer surface
    wm_pointer_set_surface(APP.GrachtClient(), nullptr, Id(), id, xOffset, yOffset);
    
    m_currentCursor = cursor;
    m_currentCursor->Subscribe(this);
    m_currentCursor->Show();
}

void Pointer::SetDefaultSurface(const DefaultCursors cursor)
{
    auto exists = m_defaultCursors.find(static_cast<int>(cursor));
    if (exists == std::end(m_defaultCursors)) {
        m_defaultCursors[static_cast<int>(cursor)] = OM.CreateClientObject<Widgets::Cursor>(
            Asgaard::APP.GetScreen(), 
            Rectangle(0, 0, CURSOR_SIZE, CURSOR_SIZE), 
            ConvertToThemeImageIndex(cursor)
        );
        exists = m_defaultCursors.find(static_cast<int>(cursor));
    }

    SetSurface((*exists).second);
}

void Pointer::Notification(const Publisher* source, const Asgaard::Notification& notification)
{
    // Is our current surface getting destroyed? 
    if (m_currentCursor && 
        notification.GetType() == NotificationType::DESTROY &&
        notification.GetObjectId() == m_currentCursor->Id()) {
        m_currentCursor.reset();
    }

    Object::Notification(source, notification);
}
