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

#include "config.hpp"
#include "object_manager.hpp"
#include "rectangle.hpp"
#include "object.hpp"
#include "pointer.hpp"
#include "utils/bitset_enum.hpp"
#include <vector>

namespace Asgaard {
    class Screen;
    class MemoryBuffer;
    class KeyEvent;
    class Pointer;
    class SubSurface;
    
    class Surface : public Object {
    public:
        enum class SurfaceEdges : unsigned int {
            NONE   = 0x0,
            TOP    = 0x1,
            BOTTOM = 0x2,
            LEFT   = 0x4,
            RIGHT  = 0x8
        };
        
        enum class FullscreenMode : int {
            EXIT,
            NORMAL,
            FULL
        };

        enum class PriorityLevel : int {
            BOTTOM,
            DEFAULT,
            TOP
        };
    public:
        ASGAARD_API Surface(uint32_t id, const std::shared_ptr<Screen>&, const Rectangle&);
        ASGAARD_API Surface(uint32_t id, const Rectangle&);
        ASGAARD_API ~Surface();
        
        ASGAARD_API void SetBuffer(const std::shared_ptr<MemoryBuffer>&);
        ASGAARD_API void MarkDamaged(const Rectangle&);
        ASGAARD_API void MarkInputRegion(const Rectangle&);
        ASGAARD_API void SetDropShadow(const Rectangle&);
        ASGAARD_API void RequestPriorityLevel(enum PriorityLevel);
        ASGAARD_API void RequestFullscreenMode(enum FullscreenMode);
        ASGAARD_API void ApplyChanges();
        
        ASGAARD_API void RequestFrame();

        ASGAARD_API void GrabPointer(const std::shared_ptr<Pointer>&);
        ASGAARD_API void UngrabPointer(const std::shared_ptr<Pointer>&);
        
        const Rectangle& Dimensions() const { return m_dimensions; }
        const std::shared_ptr<Screen>& GetScreen() const { return m_screen; }
        bool             IsFocused() const { return m_isFocused; }
        
    public:
        ASGAARD_API void ExternalEvent(const Event&) override;

    public:
        template<class S, typename... Params>
        static std::shared_ptr<S> Create(Params... parameters) {
            if (!std::is_base_of<Surface, S>::value) {
                return nullptr;
            }
            return OM.CreateClientObject<S, Params...>(std::forward<Params>(parameters)...);
        }
        
    protected:
        virtual void OnResized(enum SurfaceEdges, int width, int height) { }
        virtual void OnFocus(bool) { }
        virtual void OnFrame() { }
        virtual void OnMouseEnter(const std::shared_ptr<Pointer>&, int localX, int localY) { }
        virtual void OnMouseLeave(const std::shared_ptr<Pointer>&) { }
        virtual void OnMouseMove(const std::shared_ptr<Pointer>&, int localX, int localY) { }
        virtual void OnMouseScroll(const std::shared_ptr<Pointer>&, int scollX, int scrollY) { }
        virtual void OnMouseClick(const std::shared_ptr<Pointer>&, enum Pointer::Buttons button, bool pressed) { }
        virtual void OnKeyEvent(const KeyEvent&) { }
        virtual void Notification(const Publisher*, const Asgaard::Notification&) override;

    private:
        void BindToScreen(const std::shared_ptr<Screen>&);
        void AddChild(const std::shared_ptr<Surface>&);
        
    private:
        Rectangle                             m_dimensions;
        std::shared_ptr<Screen>               m_screen;
        std::vector<std::shared_ptr<Surface>> m_children;
        bool                                  m_isFocused;

        // allow certain accesses
        friend class SubSurface;
    };
}

// Enable bitset operations for the anchors enum
template<>
struct enable_bitmask_operators<Asgaard::Surface::SurfaceEdges> {
    static const bool enable = true;
};
