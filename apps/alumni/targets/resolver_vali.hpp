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

class ResolverVali : public ResolverBase, public Asgaard::Utils::DescriptorListener, public std::enable_shared_from_this<ResolverVali> {
public:
    ResolverVali(int stdoutDescriptor, int stderrDescriptor);
    ~ResolverVali();

public:
    bool HandleKeyCode(const Asgaard::KeyEvent&) override;
    void PrintCommandHeader() override;
    void DescriptorEvent(int iod, unsigned int events) override;

protected:
    bool CommandResolver(const std::string&, const std::vector<std::string>&) override;
    bool ChangeDirectory(const std::vector<std::string>&) override;
    std::vector<ResolverBase::DirectoryEntry> GetDirectoryContents(const std::string& Path) override;

private:
    bool ExecuteProgram(const std::string&, const std::vector<std::string>&);
    bool IsProgramPathValid(const std::string&);
    void UpdateWorkingDirectory();
    void WaitForProcess();
    
private:
    std::string m_profile;
    UUId_t      m_application;
    int         m_stdoutDescriptor;
    int         m_stdinDescriptor;
    int         m_stderrDescriptor;
    int         m_processEvent;
};
