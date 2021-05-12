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

#include <asgaard/subsurface.hpp>
#include <memory>
#include <string>

class Launcher : public SubSurface {
public:
    Launcher(uint32_t id, const std::shared_ptr<Screen>& screen, const Surface* parent, const Rectangle&);
    ~Launcher();

    void Destroy() override;

private:
    void Notification(Publisher*, int = 0, void* = 0) override;
    void OnMouseEnter(const std::shared_ptr<Pointer>&, int localX, int localY) override;
    void OnMouseLeave(const std::shared_ptr<Pointer>&) override;
    void OnMouseClick(const std::shared_ptr<Pointer>&, enum Pointer::Buttons button, bool pressed) override;

private:
    std::shared_ptr<Asgaard::MemoryPool>   m_memory;
    std::shared_ptr<Asgaard::MemoryBuffer> m_buffer;
};
