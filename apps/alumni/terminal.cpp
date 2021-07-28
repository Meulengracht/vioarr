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
#include <cstdarg>

#ifdef MOLLENOS
#include <io.h>
#include <ioset.h>
#include <os/keycodes.h>
#elif defined(_WIN32)

#else
#include <unistd.h>
#endif

#include <asgaard/object_manager.hpp>
#include <asgaard/events/key_event.hpp>
#include <asgaard/theming/theme_manager.hpp>
#include <asgaard/theming/theme.hpp>
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
    , m_command("")
    , m_commandIndex(0)
    , m_temporaryLine(nullptr)
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
    m_lines[0]->SetCursorPosition(0);

    // borrow colors from theme
    const auto theme = Asgaard::Theming::TM.GetTheme();

    // initialize text state
    m_textState.m_fgColor = m_textState.m_defaultFgColor = theme->GetColor(Asgaard::Theming::Theme::Colors::DECORATION_TEXT);
    m_textState.m_bgColor = m_textState.m_defaultBgColor = theme->GetColor(Asgaard::Theming::Theme::Colors::DEFAULT_FILL);

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
        CommitLine(true); // commit the line
        m_lines[m_inputLineIndexCurrent]->AddInput(character); // add the input
    }
    else {
        ScrollToBottom(true);
    }
}

void Terminal::RemoveInput()
{
    // remove from our copy
    if (!m_command.size()) {
        return; // nothing to remove
    }
    m_command.erase(m_command.size() - 1, 1);

    ScrollToBottom(true);
    if (!m_lines[m_inputLineIndexCurrent]->RemoveInput()) {
        if (m_inputLineIndexCurrent > m_inputLineIndexStart) {
            UndoLine(); // go one line back
            m_lines[m_inputLineIndexCurrent]->RemoveInput(); // remove an input
        }
    }
}

std::string Terminal::ClearInput(bool addToHistory)
{
    auto command = m_command;

    if (addToHistory) {
        CommandHistoryAdd(command);
    }
    CommitLine(true);

    // reset current to be start
    m_inputLineIndexStart = m_inputLineIndexCurrent;
    return command;
}

void Terminal::SetInput(const std::string& input)
{
    // update the stored command input to match this
    m_command = input;
    
    m_lines[m_inputLineIndexCurrent]->SetInput(input);
}

void Terminal::CommitLine(bool isInput)
{
    m_history.push_back(std::make_shared<TerminalLineHistory>(m_lines[m_inputLineIndexCurrent]));
    m_historyIndex = m_history.size();

    /**
     * When we commit a new line we need to handle both overflow, current input
     * and the fact that we keep track of input line count. This adds a lot of complexity
     * to the current state.
     * Lets take an example with 5 lines
     * 
     * m_rows = 5
     * 
     * 0. philip@path~ ls                             ClearInput => m_inputLineIndexStart=0, m_inputLineIndexCurrent=0
     * 1. file0 file1 file2 file3                     CommitLine => m_inputLineIndexStart=1, m_inputLineIndexCurrent=1
     * 2. philip@path~ echo test                      ClearInput => m_inputLineIndexStart=2, m_inputLineIndexCurrent=2
     * 3. test                                        CommitLine => m_inputLineIndexStart=3, m_inputLineIndexCurrent=3
     * 4. philip@path~ test test test test test test  ClearInput => m_inputLineIndexStart=4, m_inputLineIndexCurrent=4
     * 
     * Before committing the last line, the current state of m_inputLineIndexStart/m_inputLineIndexCurrent
     * should be that they should be equal, otherwise ScrollToLine will calculate wrong values. The last line
     * will trigger ScrollToLine(true), due to m_inputLineIndexCurrent == m_rows - 1.
     * 
     */

    // Are we at the end?
    if (m_inputLineIndexCurrent == m_rows - 1) {
        /**
         * If we are end of screen, we need to take into account whether or not
         * the input is multilined, then we need to reduce the inputLineIndexStart
         */
        if (isInput) {
            m_inputLineIndexStart--;
        }
        ScrollToLine(m_historyIndex, true);
    }
    else {
        m_lines[m_inputLineIndexCurrent]->SetCursorPosition(-1);
        m_inputLineIndexCurrent++;
        if (!isInput) {
            m_inputLineIndexStart++;
        }
        m_lines[m_inputLineIndexCurrent]->SetCursorPosition(0);
    }
}

void Terminal::UndoLine()
{
    m_history.pop_back();

    m_lines[m_inputLineIndexCurrent]->Reset();
    m_inputLineIndexCurrent--;
    m_lines[m_inputLineIndexCurrent]->SetCursorPosition(m_lines[m_inputLineIndexCurrent]->GetCurrentLength());
}

void Terminal::ScrollToLine(int line, bool keepInputLine)
{
    // If clear input is given, we need the last row free
    auto clearCount    = m_rows - (keepInputLine ? 1 : 0);
    auto historyStart  = line - clearCount;
    for (int i = 0; i < clearCount; i++, historyStart++) {
        m_lines[i]->Reset(m_history[historyStart]->GetCells());
    }

    if (keepInputLine) {
        m_lines[m_rows - 1]->Reset();
        m_lines[m_rows - 1]->SetCursorPosition(0);
    }
}

void Terminal::ScrollToBottom(bool keepInputLine)
{
    // History must be longer than the number of rows - 1
    auto historySize = static_cast<int>(m_history.size());
    if (historySize > m_rows && m_historyIndex < historySize) {
        m_historyIndex = historySize;
        ScrollToLine(m_historyIndex, keepInputLine);
        if (keepInputLine) {
            RestoreInputLine();
        }
    }
}

void Terminal::HistoryNext()
{
    auto historySize = static_cast<int>(m_history.size());

    // History must be longer than the number of rows - 1
    if (historySize > m_rows && m_historyIndex < historySize) {
        m_historyIndex++;

        auto atInputLine = m_historyIndex == historySize;
        ScrollToLine(m_historyIndex, atInputLine);
        if (atInputLine) {
            RestoreInputLine();
        }
    }
}

void Terminal::HistoryPrevious()
{
    // History must be longer than the number of rows
    if (m_historyIndex > m_rows) {
        auto historySize = static_cast<int>(m_history.size());
        auto atInputLine = m_historyIndex == historySize;
        if (atInputLine) {
            SaveInputLine();
        }

        m_historyIndex--;
        ScrollToLine(m_historyIndex, false);
    }
}

void Terminal::CommandHistoryPrevious()
{
    // Reset input
    m_inputLineIndexCurrent = m_inputLineIndexStart;

    // Load previous (if any)
    if (m_commandIndex > 0) {
        m_commandIndex--;
        m_command = m_commandHistory[m_commandIndex];
        m_lines[m_inputLineIndexCurrent]->SetInput(m_commandHistory[m_commandIndex]);
    }
}

void Terminal::CommandHistoryNext()
{
    // Reset input
    m_inputLineIndexCurrent = m_inputLineIndexStart;

    // Load next (if any)
    if (m_commandIndex < (static_cast<int>(m_commandHistory.size()) - 1)) {
        m_commandIndex++;
        m_command = m_commandHistory[m_commandIndex];
        m_lines[m_inputLineIndexCurrent]->SetInput(m_commandHistory[m_commandIndex]);
    }
}

void Terminal::CommandHistoryAdd(const std::string& command)
{
    if (command.size() > 0) {
        m_commandHistory.push_back(command);
        m_commandIndex = m_commandHistory.size();
        m_command = "";
    }
}

void Terminal::MoveCursorLeft()
{
    auto currentPosition = m_lines[m_inputLineIndexCurrent]->GetCursorPosition();
    if (currentPosition != 0) {
        m_lines[m_inputLineIndexCurrent]->SetCursorPosition(currentPosition - 1);
    }
}

void Terminal::MoveCursorRight()
{
    auto currentPosition = m_lines[m_inputLineIndexCurrent]->GetCursorPosition();
    if (currentPosition < m_cellWidth) {
        m_lines[m_inputLineIndexCurrent]->SetCursorPosition(currentPosition + 1);
    }
}

void Terminal::SaveInputLine()
{
    // line already stored!
    if (m_temporaryLine) {
        return;
    }

    m_temporaryLine = std::make_unique<TerminalLine>(m_font, 0, m_cellWidth);
    m_lines[m_inputLineIndexCurrent]->CloneTo(*(m_temporaryLine.get()));
}

void Terminal::RestoreInputLine()
{
    // no line stored!
    if (!m_temporaryLine) {
        return;
    }

    m_temporaryLine->CloneTo(*(m_lines[m_inputLineIndexCurrent].get()));
    m_temporaryLine.reset();
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
            CommitLine(false);
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
                CommitLine(false);
                i--;
            }
        }
    }
    RequestRedraw();
}

/**
 * Window Handling Code
 */
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
            bytesRead = read(iod, &inputBuffer[0], sizeof(inputBuffer) - 1);
        }
    }
}

void Terminal::RequestRedraw()
{
    bool shouldRedraw = m_redrawReady.exchange(false);
    if (shouldRedraw) {
        Redraw(m_buffer);
        MarkDamaged(Dimensions());
        ApplyChanges();
    }
    else {
        m_redraw = true;
    }
}

void Terminal::Redraw(const std::shared_ptr<Asgaard::MemoryBuffer>& buffer)
{
    for (int i = 0; i < m_rows; i++) {
        m_lines[i]->Redraw(buffer);
    }
}

void Terminal::PrepareBuffer(const std::shared_ptr<Asgaard::MemoryBuffer>& buffer)
{
    Asgaard::Drawing::Painter paint(buffer);
    
    const auto theme = Asgaard::Theming::TM.GetTheme();
    paint.SetFillColor(theme->GetColor(Asgaard::Theming::Theme::Colors::DEFAULT_FILL));
    paint.RenderFill();
}

void Terminal::OnCreated()
{
    // Don't hardcode 4 bytes per pixel, this is only because we assume a format of ARGB32
    auto screenSize = GetScreen()->GetCurrentWidth() * GetScreen()->GetCurrentHeight() * 4;
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
    PrepareBuffer(m_buffer);
    SetBuffer(m_buffer);
    SetDropShadow(Asgaard::Rectangle(-10, -10, 20, 30));
    m_resolver->PrintCommandHeader();
}

void Terminal::OnRefreshed(const Asgaard::MemoryBuffer*)
{
    // Request redraw
    if (m_redraw) {
        Redraw(m_buffer);
        MarkDamaged(Dimensions());
        ApplyChanges();
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
    
    if (key.KeyCode() == VKC_BACK) {
        RemoveInput();
        RequestRedraw();
    }
    else if (key.KeyCode() == VKC_ENTER) {
        std::string input = ClearInput();
        if (!m_resolver->Interpret(input)) {
            if (input.length() != 0 && m_resolver->GetClosestMatch().length() != 0) {
                Print("Command did not exist, did you mean %s?\n", m_resolver->GetClosestMatch().c_str());
            }
        }
        m_resolver->PrintCommandHeader();
    }
    else if (key.KeyCode() == VKC_UP) {
        if (key.Control()) {
            HistoryPrevious();
        }
        else {
            CommandHistoryPrevious();
        }
        RequestRedraw();
    }
    else if (key.KeyCode() == VKC_DOWN) {
        if (key.Control()) {
            HistoryNext();
        }
        else {
            CommandHistoryNext();
        }
        RequestRedraw();
    }
    else if (key.KeyCode() == VKC_LEFT) {
        MoveCursorLeft();
        RequestRedraw();
    }
    else if (key.KeyCode() == VKC_RIGHT) {
        MoveCursorRight();
        RequestRedraw();
    }
    else if (key.Key() != 0) {
        AddInput(static_cast<int>(key.Key()));
        RequestRedraw();
    }
}

void Terminal::OnResized(enum SurfaceEdges, int width, int height)
{
    // calculate new metrics
    auto rows = ((height - ALUMNI_MARGIN_TOP) / m_font->GetFontHeight()) - 1;
    auto cols = (width - (ALUMNI_MARGIN_LEFT + ALUMNI_MARGIN_RIGHT)) / m_font->GetFontWidth();

    // create a new buffer object of the requested size
    auto buffer = Asgaard::MemoryBuffer::Create(this, m_memory, 0, width,
        height, Asgaard::PixelFormat::X8B8G8R8, Asgaard::MemoryBuffer::Flags::NONE);
    
    PrepareBuffer(m_buffer);
    if (rows != m_rows || cols != m_cellWidth) {
        // update stored metrics
        m_rows = rows;
        m_cellWidth = cols;

        // clear lines
        m_lines.clear();
        for (int i = 0; i < m_rows; i++) {
            auto line = std::make_unique<TerminalLine>(m_font, i, m_cellWidth);
        }
        ScrollToLine(m_historyIndex, m_historyIndex == static_cast<int>(m_history.size()));
    }
    Redraw(buffer);
    SetBuffer(buffer);
    m_buffer->Destroy();
    m_buffer = buffer;
}

void Terminal::OnFocus(bool focus)
{
    if (focus) {
        auto currentLength = m_lines[m_inputLineIndexCurrent]->GetCurrentLength();
        m_lines[m_inputLineIndexCurrent]->SetCursorPosition(currentLength);
    }
    else {
        m_lines[m_inputLineIndexCurrent]->SetCursorPosition(-1);
    }
    RequestRedraw();
}
