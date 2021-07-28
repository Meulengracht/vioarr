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

#include <asgaard/dispatcher.hpp>
#include <asgaard/notifications/timeout_notification.hpp>
#include <algorithm>

namespace Asgaard {

Dispatcher DIS;

Dispatcher::Dispatcher()
{

}

Dispatcher::~Dispatcher()
{
    m_timers.clear();
}

void Dispatcher::Sort()
{
    m_timers.sort([] (const Timer& a, const Timer& b) {
        return a.timeLeft > b.timeLeft;
    });
}

void Dispatcher::StartTimer(Subscriber* who, int timerId, unsigned int milliseconds, bool recurring)
{
    std::lock_guard<std::mutex> guard(m_lock);
    auto existingTimer = std::find_if(std::begin(m_timers), std::end(m_timers), [who, timerId] (const Timer& a) {
        return a.who == who && a.id == timerId;
    });

    if (existingTimer != std::end(m_timers)) {
        // update timer? how much?
        existingTimer->interval = milliseconds;
    }
    else {
        Timer timer{ timerId, who, recurring, milliseconds, milliseconds, std::chrono::high_resolution_clock::now() };
        m_timers.push_back(timer);
    }
    Sort();
}

void Dispatcher::CancelTimer(Subscriber* who, int timerId)
{
    std::lock_guard<std::mutex> guard(m_lock);
    auto timer = std::find_if(std::begin(m_timers), std::end(m_timers), [who, timerId] (const Timer& a) {
        return a.who == who && a.id == timerId;
    });
    if (timer != std::end(m_timers)) {
        m_timers.erase(timer);
    }
}

unsigned int Dispatcher::Tick()
{
    std::lock_guard<std::mutex> guard(m_lock);

    auto now = std::chrono::high_resolution_clock::now();
    auto itr = std::begin(m_timers);
    while (itr != std::end(m_timers)) {
        auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(now - itr->lastUpdate);
        itr->timeLeft -= std::min(static_cast<unsigned int>(timePassed.count()), itr->timeLeft);
        itr->lastUpdate = now;
        if (itr->timeLeft) {
            // not yet time to execute
            itr = std::next(itr);
            continue;
        }

        itr->who->Notification(nullptr, TimeoutNotification(0, itr->id));
        if (itr->recurring) {
            itr->timeLeft = itr->interval;
            itr = std::next(itr);
        }
        else {
            itr = m_timers.erase(itr);
        }
    }

    Sort();
    return m_timers.size() ? m_timers.begin()->timeLeft : 0;
}

}
