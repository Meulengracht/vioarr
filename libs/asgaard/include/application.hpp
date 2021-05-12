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
#include "window_base.hpp"

typedef struct gracht_client gracht_client_t;

namespace Asgaard {
    namespace Utils {
        class DescriptorListener;
    }
    
    class Application final : public Object {
    public:
        Application();
        ~Application();
        
        /**
         * Initialize
         * Initializes the asgaard application environment, and must be invoked before any other
         * call made in this class. 
         * @throw ApplicationException
         */
        ASGAARD_API void Initialize();

        /**
         * Destroy
         * Handles application cleanup. This should not be invoked manually, will automatically be called
         * upon exit of the application
         */
        void Destroy() override;

        /**
         * AddEventDescriptor
         * Adds a c-type io descriptor like a socket, pipe or file to listen for events on. 
         * DescriptorEvent callback is then invoked in the window-class given when an event occurs.
         * @param iod The C-io descriptor that should be listening for events on
         * @param events The events to listen for, defined in <ioset.h>
         * @throw ApplicationException
         */
        ASGAARD_API void AddEventDescriptor(int iod, unsigned int events, const std::shared_ptr<Utils::DescriptorListener>&);

        /**
         * PumpMessages
         * Can be used to handle all currently queued application messages. This emulates a single
         * loop in Execute - and enables users to run this application "on-demand".
         */
        ASGAARD_API void PumpMessages();

        /**
         * Execute
         * Starts the applications main loop. The application will run untill shutdown has been requested
         * or any fault happens (caught exception).
         * @return Status code of execution
         */
        ASGAARD_API int Execute();

    public:
        gracht_client_t*               GrachtClient() const { return m_client; }
        const std::shared_ptr<Screen>& GetScreen() const    { return m_screens.front(); }

    public:
        void ExternalEvent(enum ObjectEvent event, void* data = 0) override;
        void Notification(Publisher*, int = 0, void* = 0) override;

    private:
        bool IsInitialized() const;

    private:
        std::map<int, std::shared_ptr<Utils::DescriptorListener>> m_listeners;
        std::list<std::shared_ptr<Screen>>                        m_screens;
        gracht_client_t*                                          m_client;
        int                                                       m_ioset;
        volatile bool                                             m_syncRecieved;
        volatile bool                                             m_screenFound;
        char*                                                     m_messageBuffer;
    };
    
    extern ASGAARD_API Application APP;
}
