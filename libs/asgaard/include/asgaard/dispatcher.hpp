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
 * ValiOS - Application Framework (Asgaard)
 *  - Contains the implementation of the application framework used for building
 *    graphical applications.
 */
#pragma once

#include "config.hpp"
#include "notifications/subscriber.hpp"
#include <list>
#include <mutex>
#include <chrono>

namespace Asgaard
{

class Dispatcher final
{
public:
    Dispatcher();
    ~Dispatcher();

    void StartTimer(Subscriber* who, int timerId, unsigned int milliseconds, bool recurring);
    void CancelTimer(Subscriber* who, int timerId);

public:
    unsigned int Tick();

private:
    using TimePoint = std::chrono::high_resolution_clock::time_point;
    struct Timer {
        int          id;
        Subscriber*  who;
        bool         recurring;
        unsigned int interval;
        unsigned int timeLeft;
        TimePoint    lastUpdate;
    };

    void Sort();

    std::mutex       m_lock;
    std::list<Timer> m_timers;
};

extern ASGAARD_API Dispatcher DIS;
}
