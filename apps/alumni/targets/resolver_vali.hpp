/* MollenOS
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
 * MollenOS Terminal Implementation (Alumnious)
 * - The terminal emulator implementation for Vali. Built on manual rendering and
 *   using freetype as the font renderer.
 */
#pragma once

#include <asgaard/utils/descriptor_listener.hpp>
#include <os/osdefs.h>
#include "resolver_base.hpp"
#include <memory>
#include <thread>

class ResolverVali : public ResolverBase, public Asgaard::Utils::DescriptorListener {
public:
    ResolverVali(int stdoutDescriptor, int stderrDescriptor);
    ~ResolverVali();

    /**
     * Due to how APP.AddEventDescriptor works we must call it after APP.Initialize, but this
     * resolver is built before this.
     */
    void Setup(const std::shared_ptr<ResolverVali>&);

public:
    bool HandleKeyCode(const Asgaard::KeyEvent&) override;
    void PrintCommandHeader() override;
    void DescriptorEvent(int iod, unsigned int events) override;

protected:
    bool CommandResolver(const std::string&, const std::vector<std::string>&) override;
    bool ListDirectory(const std::vector<std::string>&) override;
    bool ChangeDirectory(const std::vector<std::string>&) override;

private:
    std::vector<std::string> GetDirectoryContents(const std::string& Path);
    bool ExecuteProgram(const std::string&, const std::vector<std::string>&);
    bool IsProgramPathValid(const std::string&);
    void UpdateWorkingDirectory();
    void WaitForProcess();
    
private:
    std::string m_profile;
    std::string m_currentDirectory;
    UUId_t      m_application;
    int         m_stdoutDescriptor;
    int         m_stdinDescriptor;
    int         m_stderrDescriptor;
    int         m_processEvent;
};
