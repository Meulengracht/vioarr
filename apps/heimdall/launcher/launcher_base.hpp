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

#include <subsurface.hpp>
#include <memory_pool.hpp>
#include <memory_buffer.hpp>
#include <notifications/textchanged_notification.hpp>
#include <drawing/painter.hpp>
#include <widgets/label.hpp>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "../effects/guassian_blur.hpp"
#include "launcher_infosearch.hpp"
#include "launcher_app.hpp"

using namespace Asgaard;

class LauncherBase : public SubSurface {
public:
    LauncherBase(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions, const Drawing::Image& background)
        : SubSurface(id, screen, dimensions)
        , m_isShown(false)
    {
        LoadResources(background);
        //LoadApplications();
    }

    ~LauncherBase() {
        Destroy();
    }

    void Toggle()
    {
        if (!m_isShown) {
            SetBuffer(m_buffer);
            ApplyChanges();
        }
        else {
            std::shared_ptr<Asgaard::MemoryBuffer> empty(nullptr);
            SetBuffer(empty);
            ApplyChanges();
        }
    }

    void Destroy() override
    {
        SubSurface::Destroy();
    }

public:
    void Notification(const Publisher* source, const Asgaard::Notification& notification) override
    {
        if (notification.GetType() == NotificationType::TEXT_CHANGED) {
            // apply new filtering based on input
            auto textNotification = static_cast<const Asgaard::TextChangedNotification&>(notification);
            ApplySearchFilter(textNotification.GetText());
        }

        // do not steal events that the underlying system needs
        SubSurface::Notification(source, notification);
    }

private:
    void LoadResources(const Drawing::Image& background)
    {
        const auto& screen = GetScreen();
        auto screenSize = screen->GetCurrentWidth() * screen->GetCurrentHeight() * 4;
        m_memory = MemoryPool::Create(this, screenSize);

        m_buffer = MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
            Dimensions().Height(), PixelFormat::X8B8G8R8, MemoryBuffer::Flags::NONE);

        // create info search
        constexpr auto SEARCHBOX_WIDTH = 428.0f;
        constexpr auto SEARCHBOX_HEIGHT = 174.0f;
        auto midX = (screen->GetCurrentWidth() / 2.0f) - (SEARCHBOX_WIDTH / 2.0f);
        m_infoSearch = SubSurface::Create<LauncherInfoSearch>(
            this, 
            Rectangle(static_cast<int>(midX), 150, 
                static_cast<int>(SEARCHBOX_WIDTH), 
                static_cast<int>(SEARCHBOX_HEIGHT))
        );

        // create font
        m_font = Drawing::FM.CreateFont(DATA_DIRECTORY "/fonts/DejaVuSansMono.ttf", 24);

        // create labels
        midX = (screen->GetCurrentWidth() / 2.0f) - (200.0f / 2.0f);
        m_appTitle = SubSurface::Create<Widgets::Label>(
            this, 
            Rectangle(static_cast<int>(midX), 350, 200, 50)
        );
        m_appTitle->SetFont(m_font);
        m_appTitle->SetText("Applications");
        m_appTitle->RequestRedraw();
        
        auto renderImage = [](const auto& buffer, const auto& image) {
            Drawing::Painter painter(buffer);
            painter.RenderImage(image);
        };

        auto blurImage = [&](const auto& buffer, const auto& image) {
            GuassianBlurEffect blurEffect;
            Drawing::Image imageCopy = image;
            auto blurredBackground = blurEffect.Apply(imageCopy, 3.0);
            renderImage(buffer, blurredBackground);
        };

        blurImage(m_buffer, background);
    }

    void LoadApplications()
    {
        // hardcode initial ones
        auto editor = SubSurface::Create<LauncherApplication>(this, Rectangle(0, 0, 64, 71));
        
        auto terminal = SubSurface::Create<LauncherApplication>(this, Rectangle(0, 0, 64, 71));
        
        auto doom = SubSurface::Create<LauncherApplication>(this, Rectangle(0, 0, 64, 71));

        // add to list
        m_registeredApps.push_back(editor);
        m_registeredApps.push_back(terminal);
        m_registeredApps.push_back(doom);

        // sort list alphabetically
        std::sort(
            m_registeredApps.begin(),
            m_registeredApps.end(), 
            [](const auto& a, const auto& b) { return a->GetName() < b->GetName(); }
        );
    }

    void ApplySearchFilter(const std::string& filter)
    {
        // iterate through all apps, if their name contains stuff then show
        //std::for_each(
        //    m_registeredApps.begin(),
        //    m_registeredApps.end(), 
        //    [filter](std::shared_ptr<LauncherApplication>& a) {
        //        if (a->GetName().find(filter) != std::string::npos) {
//
        //        }
        //        else {
        //            a->
        //        }
        //    }
        //);
    }
    
private:
    std::shared_ptr<Asgaard::MemoryPool>              m_memory;
    std::shared_ptr<Asgaard::MemoryBuffer>            m_buffer;
    std::shared_ptr<Drawing::Font>                    m_font;
    std::shared_ptr<Widgets::Label>                   m_appTitle;
    std::shared_ptr<LauncherInfoSearch>               m_infoSearch;
    std::vector<std::shared_ptr<LauncherApplication>> m_registeredApps;
    bool m_isShown;
};
