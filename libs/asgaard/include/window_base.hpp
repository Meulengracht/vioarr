/**
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
#include <vector>
#include "config.hpp"
#include "pixel_format.hpp"
#include "rectangle.hpp"
#include "surface.hpp"
#include "screen.hpp"

namespace Asgaard {
    class MemoryPool;
    class MemoryBuffer;
    class WindowDecoration;
    class WindowEdge;

    namespace Drawing {
        class Image;
    }
    
    class WindowBase : public Surface {
    public:
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
        ASGAARD_API WindowBase(uint32_t, const std::shared_ptr<Screen>&, const Rectangle&);
        ASGAARD_API ~WindowBase();

        ASGAARD_API void SetTitle(const std::string& title);
        ASGAARD_API void SetIconImage(const std::shared_ptr<Drawing::Image>& image);

        ASGAARD_API void SetResizable(bool resizeable);
        ASGAARD_API void EnableDecoration(bool enable);

        ASGAARD_API void RequestPriorityLevel(enum PriorityLevel);
        ASGAARD_API void RequestFullscreenMode(enum FullscreenMode);

        ASGAARD_API void Destroy() override;
        
    // Window events that can/should be reacted on.
    protected:
        virtual void OnMinimize() { }
        virtual void OnMaximize() { }
        virtual void OnCreated() = 0;
        virtual void OnRefreshed(MemoryBuffer*) = 0;

    // Window functions that can be called to configure this window 
    protected:
        ASGAARD_API void InitiateResize(const std::shared_ptr<Pointer>&, enum SurfaceEdges);
        ASGAARD_API void InitiateMove(const std::shared_ptr<Pointer>&);

        // Protected function, allow override
        ASGAARD_API void Notification(Publisher*, int = 0, void* = 0) override;

    private:
        ASGAARD_API void ExternalEvent(enum ObjectEvent event, void* data = 0) final;

    private:
        std::vector<enum PixelFormat>     m_supportedFormats;
        std::shared_ptr<WindowDecoration> m_decoration;
        std::shared_ptr<WindowEdge>       m_edge;
    };
}
