/**
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
#include "../drawing/image.hpp"
#include <string>

namespace Asgaard {
    class MemoryPool;
    class MemoryBuffer;
    namespace Drawing {
        class Font;
    }

    namespace Widgets {
        class Textbox : public SubSurface {
        public:
            enum class Anchor {
                RIGHT,
                LEFT
            };
        public:
            ASGAARD_API Textbox(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle&);
            ~Textbox();

            ASGAARD_API void Destroy() override;
            
            ASGAARD_API void SetBackgroundColor(const Drawing::Color& color);
            ASGAARD_API void SetTextColor(const Drawing::Color& color);
            ASGAARD_API void SetFont(const std::shared_ptr<Drawing::Font>& font);
            ASGAARD_API void SetText(const std::string& text);
            ASGAARD_API void SetPlaceholderText(const std::string& text);
            ASGAARD_API void SetImage(const Drawing::Image& image);
            ASGAARD_API void SetImageAnchor(const Anchor anchor);
            ASGAARD_API void SetBorder(const Drawing::Color& color);
            ASGAARD_API void RequestRedraw();
            
        protected:
            void Notification(const Publisher*, const Asgaard::Notification&) override;
            void OnMouseEnter(const std::shared_ptr<Pointer>& pointer, int localX, int localY) override;
            void OnMouseLeave(const std::shared_ptr<Pointer>&) override;
            void OnKeyEvent(const KeyEvent& keyEvent) override;
            void OnFocus(bool) override;
            
        private:
            void CalculateBeamOffset();
            void Redraw();
            void RedrawReady();
            void OnBackspace();
            void OnDelete();
            void OnLeftArrow();
            void OnRightArrow();
            void AddInput(const KeyEvent& keyEvent);
    
        private:
            std::shared_ptr<Asgaard::MemoryPool>   m_memory;
            std::shared_ptr<Asgaard::MemoryBuffer> m_buffer;
            std::shared_ptr<Drawing::Font>         m_font;

            int            m_cursor;
            int            m_beamOffset;
            std::string    m_text;
            std::string    m_placeholderText;
            Drawing::Color m_textColor;
            Drawing::Color m_placeholderTextColor;
            Drawing::Color m_fillColor;
            Drawing::Color m_borderColor;
            int            m_borderWidth;
            Anchor         m_imageAnchor;
            Drawing::Image m_image;
        
            bool              m_redraw;
            std::atomic<bool> m_redrawReady;
        };
    }
}
