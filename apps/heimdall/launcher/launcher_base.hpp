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
#include <theming/theme_manager.hpp>
#include <theming/theme.hpp>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "../effects/guassian_blur.hpp"
#include "../utils/spawner.hpp"
#include "launcher_infosearch.hpp"
#include "launcher_app.hpp"

using namespace Asgaard;

constexpr auto SEARCHBOX_WIDTH = 428.0f;
constexpr auto SEARCHBOX_HEIGHT = 174.0f;
constexpr auto TILE_WIDTH = 64.0f;
constexpr auto TILE_HEIGHT = 78.0f;
constexpr auto TILE_PADDING = 30;

class LauncherBase : public Surface {
public:
    LauncherBase(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions, const Drawing::Image& background)
        : Surface(id, screen, dimensions)
        , m_applicationBox(0, 0, 0, 0)
        , m_tileBox(0, 0, static_cast<int>(TILE_WIDTH), static_cast<int>(TILE_HEIGHT))
        , m_tilesPerRow(0)
        , m_isShown(false)
    {
        LoadResources(background);
        CalculateDimensions();
        LoadApplications();
        Configure();
    }

    ~LauncherBase() {
        Destroy();
    }

    void Toggle()
    {
        if (!m_isShown) {
            ApplySearchFilter("");
            SetBuffer(m_buffer);
            ApplyChanges();
            m_infoSearch->OnShown();
        }
        else {
            std::shared_ptr<Asgaard::MemoryBuffer> empty(nullptr);
            SetBuffer(empty);
            ApplyChanges();
        }

        m_isShown = !m_isShown;
    }

    void Destroy() override
    {
        Surface::Destroy();
    }

public:
    void Notification(const Publisher* source, const Asgaard::Notification& notification) override
    {
        auto spawn = [&] (uint32_t objectId) {
            auto hasInfo = m_registeredAppInfos.find(objectId);
            if (hasInfo != std::end(m_registeredAppInfos)) {
                Toggle();
                Spawner::SpawnApplication((*hasInfo).second);
            }
        };

        if (notification.GetType() == NotificationType::TEXT_CHANGED) {
            // apply new filtering based on input
            auto textNotification = static_cast<const Asgaard::TextChangedNotification&>(notification);
            ApplySearchFilter(textNotification.GetText());
        }
        else if (notification.GetType() == NotificationType::TEXT_COMMIT) {
            // launch first app
            auto app = std::find_if(
                std::begin(m_registeredApps),
                std::end(m_registeredApps),
                [] (const auto& a) { return a->IsShown(); }
            );

            if (app != std::end(m_registeredApps)) {
                spawn((*app)->Id());
            }
        }
        else if (notification.GetType() == NotificationType::CLICKED) {
            spawn(notification.GetObjectId());
        }

        // do not steal events that the underlying system needs
        Surface::Notification(source, notification);
    }

    void CalculateDimensions()
    {
        // We want the application box to be a little bit larger than the
        // infosearch box
        auto width = SEARCHBOX_WIDTH + (2 * (TILE_WIDTH + TILE_PADDING));
        auto height = (3 * TILE_HEIGHT) + (2 * TILE_PADDING);
        auto x = (GetScreen()->GetCurrentWidth() / 2.0f) - (width / 2.0f);
        auto y = 450;

        m_applicationBox = Rectangle(
            static_cast<int>(x), y, 
            static_cast<int>(width),
            static_cast<int>(height)
        );
        m_tilesPerRow = static_cast<int>(width / TILE_WIDTH);
    }

    void LoadResources(const Drawing::Image& background)
    {
        const auto& screen = GetScreen();
        auto screenSize = screen->GetCurrentWidth() * screen->GetCurrentHeight() * 4;
        m_memory = MemoryPool::Create(this, screenSize);

        m_buffer = MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
            Dimensions().Height(), PixelFormat::X8B8G8R8, MemoryBuffer::Flags::NONE);

        // create info search
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

    void RegisterApplication(const std::string& name, const Drawing::Image& image, const std::string& path)
    {
        // create the app visual
        auto index = m_registeredApps.size();
        auto app   = SubSurface::Create<LauncherApplication>(this, 
            Rectangle(GetApplicationTileX(index), GetApplicationTileY(index),
                static_cast<int>(TILE_WIDTH), static_cast<int>(TILE_HEIGHT)), 
            name, image
        );
        m_registeredApps.push_back(app);

        // create the app info
        m_registeredAppInfos.insert(std::make_pair(app->Id(), path));
    }

    void LoadApplications()
    {
        const auto theme = Theming::TM.GetTheme();

        auto termImage = theme->GetImage(Theming::Theme::Elements::IMAGE_TERMINAL);
        auto editImage = theme->GetImage(Theming::Theme::Elements::IMAGE_EDITOR);
        auto gameImage = theme->GetImage(Theming::Theme::Elements::IMAGE_GAME);

        // hardcode initial ones
#ifdef MOLLENOS
        RegisterApplication("Terminal", termImage, "$bin/alumni.app");
        RegisterApplication("Lite", editImage, "$bin/lite.app");
        RegisterApplication("Doom", gameImage, "$bin/doom.app");
#else
        RegisterApplication("Terminal", termImage, "alumni");
        RegisterApplication("Lite", editImage, "alumni");
        RegisterApplication("Doom", gameImage, "alumni");
#endif

        // sort list alphabetically
        std::sort(
            m_registeredApps.begin(),
            m_registeredApps.end(), 
            [](const auto& a, const auto& b) { return a->GetName() < b->GetName(); }
        );
    }

    void Configure()
    {
        RequestPriorityLevel(PriorityLevel::TOP);
        RequestFullscreenMode(FullscreenMode::NORMAL);
        MarkInputRegion(Dimensions());
    }

    void ApplySearchFilter(const std::string& filter)
    {
        // iterate through all apps, if their name contains stuff then show
        auto i = 0;
        std::for_each(
            m_registeredApps.begin(),
            m_registeredApps.end(), 
            [this, filter, &i](std::shared_ptr<LauncherApplication>& a) {
                if (filter.size() == 0 || a->GetName().find(filter) != std::string::npos) {
                    a->Move(this->GetApplicationTileX(i), this->GetApplicationTileY(i));
                    a->Show();
                    a->SetHighlight(!i && filter.size() > 0);
                    a->RequestRedraw();
                    i++;
                }
                else {
                    a->SetHighlight(false);
                    a->Hide();
                }
            }
        );
    }

    int GetApplicationTileX(int tileIndex)
    {
        auto rowIndex = tileIndex % m_tilesPerRow;
        auto xCoord   = m_applicationBox.X() + (rowIndex * (m_tileBox.Width() + TILE_PADDING));
        return xCoord;
    }

    int GetApplicationTileY(int tileIndex)
    {
        auto row    = tileIndex / m_tilesPerRow;
        auto yCoord = m_applicationBox.Y() + (row * (m_tileBox.Height() + TILE_PADDING));
        return yCoord;
    }
    
private:
    std::shared_ptr<Asgaard::MemoryPool>              m_memory;
    std::shared_ptr<Asgaard::MemoryBuffer>            m_buffer;
    std::shared_ptr<Drawing::Font>                    m_font;
    std::shared_ptr<Widgets::Label>                   m_appTitle;
    std::shared_ptr<LauncherInfoSearch>               m_infoSearch;
    std::vector<std::shared_ptr<LauncherApplication>> m_registeredApps;
    std::map<uint32_t, std::string>                   m_registeredAppInfos;

    Rectangle m_applicationBox;
    Rectangle m_tileBox;
    int       m_tilesPerRow;

    bool m_isShown;
};
