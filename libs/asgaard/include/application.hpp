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

#include "config.hpp"
#include "object.hpp"
#include <map>
#include <memory>

typedef struct gracht_client gracht_client_t;

namespace Asgaard {
    class Keyboard;
    class Pointer;
    class Screen;

    namespace Utils {
        class DescriptorListener;
    }
    
    class Application final : public Object {
    public:
        /**
         * Most of these applications have very specific purposes and does never
         * really need to be manipulated. Some of these should be changed only if
         * you've been instructed to do so.
         */
        enum class Settings : int {
            ASYNC_DESCRIPTOR, // Accepts a pointer to a descriptor.
            HEIMDALL_VISIBLE, // Accepts a bool value; Wheter or not application is registered with Heimdall
            APPLICATION_GUID, // Accepts a pointer to a std::string instance; sets the GUID of the application
            APPLICATION_ICON  // Accepts a pointer to a std::string instance; controls which application icon is showed.
        };
    public:
        Application();
        ~Application();
        
        /**
         * Initializes the asgaard application environment, and must be invoked before any other
         * call made in this class. 
         * @throw ApplicationException
         */
        ASGAARD_API void Initialize();

        /**
         * Adds a c-type io descriptor like a socket, pipe or file to listen for events on. 
         * DescriptorEvent callback is then invoked in the window-class given when an event occurs.
         * @param iod The C-io descriptor that should be listening for events on
         * @param events The events to listen for, defined in <ioset.h>
         * @throw ApplicationException
         */
        ASGAARD_API void AddEventDescriptor(int iod, unsigned int events, const std::shared_ptr<Utils::DescriptorListener>&);
        ASGAARD_API void SetEventListener(int iod, const std::shared_ptr<Utils::DescriptorListener>&);

        /**
         * Can be used to handle all currently queued application messages. This emulates a single
         * loop in Execute - and enables users to run this application "on-demand".
         */
        ASGAARD_API void PumpMessages();

        /**
         * Starts the applications main loop. The application will run untill shutdown has been requested
         * or any fault happens (caught exception).
         * @return Status code of execution
         */
        ASGAARD_API int Execute();

        /**
         * Sets a new value for the given setting. This can be used before a call to Initialize to configure
         * some OS-specific startup parameters. This cannot be called after Initialize() is called.
         * 
         * @param setting The setting that should be configured to the provided value.
         * @param value   The new value of the setting, or nullptr to clear the setting.
         */
        ASGAARD_API void  SetSetting(Settings setting, void* value);

        template<typename T>
        void SetSettingValue(Settings setting, T value)
        {
            auto asValue = static_cast<std::uintptr_t>(value);
            SetSetting(setting, reinterpret_cast<void*>(asValue));
        }

        /**
         * Retrieves the value of the specified setting.
         * 
         * @param setting The setting to retrieve the value of.
         * @return void*  The value of the setting, or nullptr if none exists. 
         */
        ASGAARD_API void* GetSetting(Settings setting);

        template<typename T>
        T GetSettingValue(Settings setting)
        {
            auto asValue = reinterpret_cast<std::uintptr_t>(GetSetting(setting));
            return static_cast<T>(asValue);
        }

        /**
         * Destroy
         * Handles application cleanup. This should not be invoked manually, will automatically be called
         * upon exit of the application
         */
        void Destroy() override;

    public:
        gracht_client_t*               VioarrClient() const { return m_client; }
        const std::shared_ptr<Screen>& GetScreen() const    { return m_screens.front(); }
        std::shared_ptr<Keyboard>      GetKeyboard() const;
        std::shared_ptr<Pointer>       GetPointer() const;

    public:
        void ExternalEvent(const Event&) override;
        void Notification(const Publisher*, const Asgaard::Notification&) override;

    private:
        bool IsInitialized() const;
        void InitializeInternal();
        void DestroyInternal();

    private:
        std::map<int, void*>                                      m_settings;
        std::map<int, std::shared_ptr<Utils::DescriptorListener>> m_listeners;
        std::list<std::shared_ptr<Screen>>                        m_screens;
        std::list<std::shared_ptr<Object>>                        m_inputs;
        gracht_client_t*                                          m_client;
        AsyncHandleType                                           m_ioset;
        volatile bool                                             m_syncRecieved;
        volatile bool                                             m_screenFound;
    };
    
    extern ASGAARD_API Application APP;
}
