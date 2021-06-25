/**
 * Copyright 2021, Philip Meulengracht
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
 * ValiOS - Application Environment (Launcher)
 *  - Contains the implementation of the application environment to support
 *    graphical user interactions.
 */
#pragma once

#include <gracht/server.h>
#include <subsurface.hpp>
#include <memory_pool.hpp>
#include <memory_buffer.hpp>
#include <notifications/textchanged_notification.hpp>
#include <drawing/painter.hpp>
#include <widgets/label.hpp>
#include <theming/theme_manager.hpp>
#include <theming/theme.hpp>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "appicon.hpp"
#include "../utils/register.hpp"

using namespace Asgaard;

constexpr auto ICON_PADDING = 16;

class ApplicationBar : public Surface {
public:
    ApplicationBar(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions)
        : Surface(id, screen, dimensions)
    {
        LoadResources();
        CalculateDimensions();
        Configure();
    }

    ~ApplicationBar() {
        Destroy();
    }

    void Destroy() override
    {
        Surface::Destroy();
    }

    void OnApplicationRegister(gracht_conn_t source, unsigned int applicationId)
    {
        auto app = Heimdall::Register::GetApplication(applicationId);
        if (!app) {
            // uh oh
            return;
        }

        auto icon = SubSurface::Create<ApplicationIcon>(m_iconDimensions, app);
        m_apps.push_back(icon);

        // recalculate positions
        RecalculateIconStart();
        UpdateIconPositions();
    }

    void OnApplicationUnregister(gracht_conn_t source, unsigned int applicationId)
    {

        // recalculate positions
        RecalculateIconStart();
        UpdateIconPositions();
    }

public:
    void LoadResources()
    {
        auto statusBarSize = Dimensions().Width() * Dimensions().Height() * 4;
        m_memory = MemoryPool::Create(this, statusBarSize);

        m_buffer = MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
            Dimensions().Height(), PixelFormat::X8B8G8R8, MemoryBuffer::Flags::NONE);

        auto render = [&] {
            Drawing::Painter painter(m_buffer);

            const auto theme = Theming::TM.GetTheme();
            painter.SetFillColor(theme->GetColor(Theming::Theme::Colors::DEFAULT_FILL));
            painter.RenderFill();
        };
        render();
    }

    void Configure()
    {
        RequestPriorityLevel(PriorityLevel::TOP);
        MarkInputRegion(Dimensions());
        SetBuffer(m_buffer);
        ApplyChanges();
    }

    void CalculateDimensions()
    {
        std::vector<int> availableSizes = { 24, 32, 48, 64 };
        auto height = Dimensions().Height();
        auto targetHeight = availableSizes[0];
        for (auto val : availableSizes) {
            if (val > height) {
                break;
            }
            targetHeight = val;
        }

        // place Y in mid
        auto midY = (Dimensions().Height() / 2) - (targetHeight / 2);

        // create NxN size
        m_iconDimensions = Rectangle(0, midY, targetHeight, targetHeight);
    }

    void RecalculateIconStart()
    {
        auto spaceRequired = m_apps.size() * (m_iconDimensions.Width() + ICON_PADDING);
        if (spaceRequired) {
            spaceRequired -= ICON_PADDING; // remove the last padding
        }

        if (spaceRequired > Dimensions().Width()) {
            // fuck what to do
            spaceRequired = Dimensions().Width();
        }

        m_startX = (Dimensions().Width() / 2) - (spaceRequired / 2);
    }

    void UpdateIconPositions()
    {
        int index = 0;
        for (const auto& app : m_apps) {
            app->Move(GetIconPositionX(index++), m_iconDimensions.Y());
            app->RequestRedraw();
        }
    }

    int GetIconPositionX(int index)
    {
        return m_startX + (index * (m_iconDimensions.Width() + ICON_PADDING));
    }

private:
    std::shared_ptr<Asgaard::MemoryPool>         m_memory;
    std::shared_ptr<Asgaard::MemoryBuffer>       m_buffer;
    std::vector<std::shared_ptr<ApplicationIcon> m_apps;
    Rectangle                                    m_iconDimensions;
    int                                          m_startX;
};
