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

#include <atomic>
#include <mutex>
#include <memory>
#include <vector>
#include <string>
#include <list>

#include <asgaard/application.hpp>
#include <asgaard/window_base.hpp>
#include <asgaard/memory_pool.hpp>
#include <asgaard/memory_buffer.hpp>
#include <asgaard/drawing/font.hpp>
#include <asgaard/drawing/painter.hpp>
#include <asgaard/drawing/color.hpp>
#include <asgaard/utils/descriptor_listener.hpp>

#define ALUMNI_MARGIN_TOP   40
#define ALUMNI_MARGIN_LEFT  10
#define ALUMNI_MARGIN_RIGHT 10

class ResolverBase;
class TerminalLine;
class TerminalLineHistory;

class Terminal : public Asgaard::WindowBase, public Asgaard::Utils::DescriptorListener {
private:
    struct TextState {
        Asgaard::Drawing::Color m_fgColor;
        Asgaard::Drawing::Color m_bgColor;


        Asgaard::Drawing::Color m_defaultFgColor;
        Asgaard::Drawing::Color m_defaultBgColor;
    };

public:
    Terminal(uint32_t id, const std::shared_ptr<Asgaard::Screen>& screen, const Asgaard::Rectangle&, 
        const std::shared_ptr<Asgaard::Drawing::Font>&,
        const std::shared_ptr<ResolverBase>&, int stdoutDescriptor, int stderrDescriptor);
    ~Terminal();

    void Print(const char* format, ...);
    void RequestRedraw();

    // Input manipulation
    std::string ClearInput(bool newline);
    void RemoveInput();
    void AddInput(int character);
    
    // History manipulation
    void HistoryPrevious();
    void HistoryNext();

    // Cursor manipulation
    void MoveCursorLeft();
    void MoveCursorRight();

    // Stats
    int GetNumberOfCellsPerLine() const { return m_cellWidth; }

protected:
    void OnCreated() override;
    void OnRefreshed(Asgaard::MemoryBuffer*) override;
    void OnKeyEvent(const Asgaard::KeyEvent&) override;
    
private:
    void Redraw();
    void PrepareBuffer();
    void CommitLine();
    void UndoLine();
    void ScrollToLine(bool clearInput);

    void   InitializeVT();
    size_t ParseVTEscapeCode(const char* buffer);
    size_t HandleVTEscapeCode(const char* buffer);

    // We do not allow override of these
    void DescriptorEvent(int iod, unsigned int events) override;

private:
    std::shared_ptr<Asgaard::MemoryPool>    m_memory;
    std::shared_ptr<Asgaard::MemoryBuffer>  m_buffer;
    std::shared_ptr<Asgaard::Drawing::Font> m_font;
    std::shared_ptr<ResolverBase>           m_resolver;
    
    struct TextState                                  m_textState;
    int                                               m_rows;
    int                                               m_cellWidth;
    std::vector<std::shared_ptr<TerminalLineHistory>> m_history;
    int                                               m_historyIndex;
    std::string                                       m_command;
    std::vector<std::string>                          m_commandHistory;
    std::vector<std::unique_ptr<TerminalLine>>        m_lines;
    int                                               m_inputLineIndexStart;
    int                                               m_inputLineIndexCurrent;
    char*                                             m_printBuffer;
    std::mutex                                        m_printLock;

    int m_stdoutDescriptor;
    int m_stderrDescriptor;

    bool m_redraw;
    std::atomic<bool> m_redrawReady;
};
