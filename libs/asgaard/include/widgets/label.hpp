/* ValiOS
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
 * ValiOS - Application Framework (Asgaard)
 *  - Contains the implementation of the application framework used for building
 *    graphical applications.
 */
#pragma once

#include <atomic>
#include <memory>
#include "../config.hpp"
#include "../utils/bitset_enum.hpp"
#include "../subsurface.hpp"
#include "../drawing/color.hpp"
#include <string>

namespace Asgaard {
    class MemoryPool;
    class MemoryBuffer;
    namespace Drawing {
        class Font;
    }

    namespace Widgets {
        class Label : public SubSurface {
        public:
            enum class Anchors {
                TOP = 0x1,
                CENTER = 0x2,
                BOTTOM = 0x4,
                RIGHT = 0x8,
                LEFT = 0x10
            };
        public:
            Label(uint32_t id, const std::shared_ptr<Screen>& screen, const Surface* parent, const Rectangle&);
            ~Label();

            void Destroy() override;
            
            void SetBackgroundColor(const Drawing::Color& color);
            void SetTextColor(const Drawing::Color& color);
            void SetFont(const std::shared_ptr<Drawing::Font>& font);
            void SetText(const std::string& text);
            void SetAnchors(Anchors anchors);
            void RequestRedraw();
            
        protected:
            void Notification(Publisher*, int = 0, void* = 0) override;
            
        private:
            void Redraw();
            void RedrawReady();
            int  CalculateXCoordinate(const Rectangle& textDimensions);
            int  CalculateYCoordinate(const Rectangle& textDimensions);
    
        private:
            std::shared_ptr<Asgaard::MemoryPool>   m_memory;
            std::shared_ptr<Asgaard::MemoryBuffer> m_buffer;
            std::shared_ptr<Drawing::Font>         m_font;

            std::string    m_text;
            Anchors        m_anchors;
            Drawing::Color m_textColor;
            Drawing::Color m_fillColor;
        
            bool              m_redraw;
            std::atomic<bool> m_redrawReady;
        };
    }
}

// Enable bitset operations for the anchors enum
template<>
struct enable_bitmask_operators<Asgaard::Widgets::Label::Anchors> {
    static const bool enable = true;
};
