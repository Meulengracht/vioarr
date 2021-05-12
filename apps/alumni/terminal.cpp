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

#include <cassert>
#include <cctype>
#include <cmath>

#include <io.h>
#include <ioset.h>
#include <os/keycodes.h>

#include <asgaard/object_manager.hpp>
#include <asgaard/key_event.hpp>
#include "terminal.hpp"
#include "terminal_line.hpp"
#include "terminal_interpreter.hpp"
#include "targets/resolver_base.hpp"

const int PRINTBUFFER_SIZE = 4096;

Terminal::Terminal(uint32_t id, const std::shared_ptr<Asgaard::Screen>& screen, const Asgaard::Rectangle& dimensions, 
    const std::shared_ptr<Asgaard::Drawing::Font>& font,
    const std::shared_ptr<ResolverBase>& resolver, int stdoutDescriptor, int stderrDescriptor)
    : WindowBase(id, screen, dimensions)
    , m_font(font)
    , m_resolver(resolver)
    , m_rows(-1)
    , m_cellWidth(0)
    , m_historyIndex(0)
    , m_inputLineIndexStart(0)
    , m_inputLineIndexCurrent(0)
    , m_stdoutDescriptor(stdoutDescriptor)
    , m_stderrDescriptor(stderrDescriptor)
    , m_redraw(false)
    , m_redrawReady(true)
{
    // Calculate row count taking into account the margins
    m_rows = ((dimensions.Height() - ALUMNI_MARGIN_TOP) / font->GetFontHeight()) - 1;
    m_printBuffer = (char*)std::malloc(PRINTBUFFER_SIZE);

    m_cellWidth = (dimensions.Width() - (ALUMNI_MARGIN_LEFT + ALUMNI_MARGIN_RIGHT)) / font->GetFontWidth();
    for (int i = 0; i < m_rows; i++) {
        m_lines.push_back(std::make_unique<TerminalLine>(font, i, m_cellWidth));
    }

    // show cursor
    m_lines[0]->ShowCursor();

    // initialize text state
    m_textState.m_fgColor = m_textState.m_defaultFgColor = Asgaard::Drawing::Color(0xFF, 0xFF, 0xFF);
    m_textState.m_bgColor = m_textState.m_defaultBgColor = Asgaard::Drawing::Color(0x7F, 0x0C, 0x35, 0x33);

    // initialize VT environment
    InitializeVT();
}

Terminal::~Terminal()
{
    std::free(m_printBuffer);
}

void Terminal::AddInput(int character)
{
    // add to our copy of command buffer
    m_command.push_back(character);

    if (!m_lines[m_inputLineIndexCurrent]->AddInput(character)) {
        CommitLine();
    }
}

void Terminal::RemoveInput()
{
    // remove from our copy
    if (!m_command.size()) {
        return; // nothing to remove
    }
    m_command.erase(m_command.size() - 1, 1);

    if (!m_lines[m_inputLineIndexCurrent]->RemoveInput()) {
        // uh todo, we should skip to prev line
        return;
    }
}

std::string Terminal::ClearInput(bool newline)
{
    auto command = m_command;
    m_commandHistory.push_back(command);
    m_command = "";

    m_inputLineIndexStart = m_inputLineIndexCurrent;
    if (newline) {
        CommitLine();
    }
    else {
        m_lines[m_inputLineIndexCurrent]->Reset();
    }
    return command;
}

void Terminal::CommitLine()
{
    m_history.push_back(std::make_shared<TerminalLineHistory>(m_lines[m_inputLineIndexCurrent]));
    m_historyIndex = m_history.size();

    // Are we at the end?
    if (m_inputLineIndexCurrent == m_rows - 1) {
        ScrollToLine(true);
    }
    else {
        m_lines[m_inputLineIndexCurrent]->HideCursor();
        m_inputLineIndexCurrent++;
        m_lines[m_inputLineIndexCurrent]->ShowCursor();
    }
}

void Terminal::UndoLine()
{
    m_history.pop_back();

    m_lines[m_inputLineIndexCurrent]->HideCursor();
    m_inputLineIndexCurrent--;
    m_lines[m_inputLineIndexCurrent]->ShowCursor();
}

void Terminal::ScrollToLine(bool clearInput)
{
    // If clear input is given, we need the last row free
    auto numInputLines = (m_inputLineIndexCurrent - m_inputLineIndexStart) + 1;
    auto clearCount    = m_rows - (clearInput ? numInputLines : 0);
    auto historyStart  = m_historyIndex - clearCount;
    for (int i = 0; i < clearCount; i++, historyStart++) {
        m_lines[i]->Reset(m_history[historyStart]->GetCells());
    }

    if (clearInput) {
        m_lines[m_rows - 1]->Reset();
        m_lines[m_rows - 1]->ShowCursor();
    }
}

void Terminal::HistoryNext()
{
    auto historySize = static_cast<int>(m_history.size());

    // History must be longer than the number of rows - 1
    if (historySize > m_rows && m_historyIndex < historySize) {
        m_historyIndex++;
        ScrollToLine(m_historyIndex == historySize);
    }
}

void Terminal::HistoryPrevious()
{
    auto historySize = static_cast<int>(m_history.size());

    // History must be longer than the number of rows
    if (m_historyIndex > m_rows) {
        m_historyIndex--;
        ScrollToLine(false);
    }
}

void Terminal::MoveCursorLeft()
{

}

void Terminal::MoveCursorRight()
{

}

void Terminal::Print(const char* format, ...)
{
    std::unique_lock<std::mutex> lockedSection(m_printLock);
    va_list arguments;
    size_t  i;

    va_start(arguments, format);
    vsnprintf(m_printBuffer, PRINTBUFFER_SIZE, format, arguments);
    va_end(arguments);

    for (i = 0; i < PRINTBUFFER_SIZE && m_printBuffer[i]; i++) {
        if (m_printBuffer[i] == '\n') {
            CommitLine();
        }
        else if (m_printBuffer[i] == '\033') { // 0x1B
            auto charsConsumed = ParseVTEscapeCode(&m_printBuffer[i]);
            if (!charsConsumed) {
                continue;
            }
            i += (charsConsumed - 1); // take i++ into account in loop body
        }
        else {
            if (!m_lines[m_inputLineIndexCurrent]->AddCharacter(m_printBuffer[i], m_textState.m_fgColor)) {
                CommitLine();
                i--;
            }
        }
    }
    RequestRedraw();
}

/**
 * Window Handling Code
 */
#define __TRACE
#include <ddk/utils.h>
void Terminal::DescriptorEvent(int iod, unsigned int events)
{
    if (iod == m_stdoutDescriptor || iod == m_stderrDescriptor) {
        char inputBuffer[256];

        // read -1 bytes to account for the fact that we should always include a zero terminator
        // which Print requires. Otherwise we can end up reading from the rest of the stack.
        int bytesRead = read(iod, &inputBuffer[0], sizeof(inputBuffer) - 1);
        while (bytesRead > 0) {
            // set zero terminator
            inputBuffer[bytesRead] = 0;
            Print("%s", &inputBuffer[0]);
            TRACE("%s", &inputBuffer[0]);
            bytesRead = read(iod, &inputBuffer[0], sizeof(inputBuffer) - 1);
        }
    }
}

void Terminal::RequestRedraw()
{
    bool shouldRedraw = m_redrawReady.exchange(false);
    if (shouldRedraw) {
        Redraw();
    }
    else {
        m_redraw = true;
    }
}

void Terminal::Redraw()
{
    for (int i = 0; i < m_rows; i++) {
        m_lines[i]->Redraw(m_buffer);
    }
    MarkDamaged(Dimensions());
    ApplyChanges();
}

void Terminal::PrepareBuffer()
{
    Asgaard::Drawing::Painter paint(m_buffer);
    
    paint.SetFillColor(0x7F, 0x0C, 0x35, 0x33);
    paint.RenderFill();
}

void Terminal::OnCreated()
{
    // Don't hardcode 4 bytes per pixel, this is only because we assume a format of ARGB32
    auto screenSize = m_screen->GetCurrentWidth() * m_screen->GetCurrentHeight() * 4;
    m_memory = Asgaard::MemoryPool::Create(this, screenSize);
    
    // Create initial buffer the size of this surface
    m_buffer = Asgaard::MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
        Dimensions().Height(), Asgaard::PixelFormat::X8B8G8R8, Asgaard::MemoryBuffer::Flags::NONE);
    
    // we couldn't do this in constructor as the OM had not registered us
    auto terminal = std::dynamic_pointer_cast<Terminal>(Asgaard::OM[Id()]);
    m_resolver->SetTerminal(terminal);

    SetTitle("Alumni Terminal v1.0-dev");
    EnableDecoration(true);

    // Now all resources are created
    PrepareBuffer();
    SetBuffer(m_buffer);
    SetDropShadow(Asgaard::Rectangle(-10, -10, 20, 30));
    m_resolver->PrintCommandHeader();
}

void Terminal::OnRefreshed(Asgaard::MemoryBuffer*)
{
    // Request redraw
    if (m_redraw) {
        Redraw();
        m_redraw = false;
    }
    else {
        m_redrawReady.store(true);
    }
}

void Terminal::OnKeyEvent(const Asgaard::KeyEvent& key)
{
    // Check if we should forward input first
    if (m_resolver->HandleKeyCode(key)) {
        return;
    }

    // Don't respond to released events
    if (!key.Pressed()) {
        return;
    }
    
    if (key.KeyCode() == VK_BACK) {
        RemoveInput();
        RequestRedraw();
    }
    else if (key.KeyCode() == VK_ENTER) {
        std::string input = ClearInput(true);
        if (!m_resolver->Interpret(input)) {
            if (m_resolver->GetClosestMatch().length() != 0) {
                Print("Command did not exist, did you mean %s?\n", m_resolver->GetClosestMatch().c_str());
            }
        }
        m_resolver->PrintCommandHeader();
    }
    else if (key.KeyCode() == VK_UP) {
        HistoryPrevious();
        RequestRedraw();
    }
    else if (key.KeyCode() == VK_DOWN) {
        HistoryNext();
        RequestRedraw();
    }
    else if (key.KeyCode() == VK_LEFT) {
        MoveCursorLeft();
        RequestRedraw();
    }
    else if (key.KeyCode() == VK_RIGHT) {
        MoveCursorRight();
        RequestRedraw();
    }
    else if (key.KeyAscii() != 0) {
        AddInput(key.KeyAscii());
        RequestRedraw();
    }
}
