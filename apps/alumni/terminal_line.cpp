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

#include "terminal_line.hpp"
#include "terminal.hpp"
#include <theming/theme_manager.hpp>
#include <theming/theme.hpp>

TerminalLine::TerminalLine(const std::shared_ptr<Asgaard::Drawing::Font>& font, int row, int initialCellCount)
    : m_font(font)
    , m_dimensions(ALUMNI_MARGIN_LEFT, (
                   row * font->GetFontHeight()) + ALUMNI_MARGIN_TOP, 
                   initialCellCount * font->GetFontWidth(), 
                   font->GetFontHeight())
    , m_cells(initialCellCount)
    , m_row(row)
{
    Reset();
}

void TerminalLine::Reset()
{
    m_text.clear();

    Asgaard::Drawing::Color color(0xFF, 0xFF, 0xFF);
    for (auto& cell : m_cells) {
        cell.m_character = 0;
        cell.m_color = color;
    }

    m_showCursor  = false;
    m_inputOffset = 0;
    m_cursor      = 0;
    m_dirty       = true;
}

void TerminalLine::Reset(const std::vector<TerminalCell>& cells)
{
    m_cells       = cells;
    m_showCursor  = false;
    m_inputOffset = 0;
    m_cursor      = 0;
    m_dirty       = true;
    RebuildText();
}

void TerminalLine::Resize(int cellCount, std::vector<TerminalCell>& overflownCells)
{
    auto capacity = static_cast<int>(m_cells.size());

    // if we reduce capacity, we must spill the cells that gets cut
    if (cellCount < capacity) {
        auto difference = capacity - cellCount;
        Spill(cellCount, difference, overflownCells);
    }

    m_cells.resize(cellCount);
}

void TerminalLine::PrependCells(const std::vector<TerminalCell>& cells, std::vector<TerminalCell>& overflownCells)
{
    auto count = cells.size();
    auto roomAvailable = m_cells.size() - m_text.size();
    if (count > roomAvailable) {
        auto difference = count - roomAvailable;
        Spill(m_text.size() - difference, difference, overflownCells);
    }

    for (const auto& cell : cells) {
        ShiftCellsRight(0);
        m_cells[0] = cell;
    }
}

int TerminalLine::AppendCells(const std::vector<TerminalCell>& cells)
{
    auto count = cells.size();
    auto index = m_text.size();
    auto roomAvailable = m_cells.size() - index;
    auto cellsAppended = 0;
    for (; cellsAppended < static_cast<int>(std::min(count, roomAvailable)); cellsAppended++, index++) {
        m_cells[index] = cells[cellsAppended];
    }
    return cellsAppended;
}

bool TerminalLine::AddCharacter(int character)
{
    return AddCharacter(character, Asgaard::Drawing::Color(0xFF, 0xFF, 0xFF));
}

bool TerminalLine::AddCharacter(int character, const Asgaard::Drawing::Color& color)
{
    if (AddInput(character, color)) {
        m_inputOffset++;
        return true;
    }
    return false;
}

bool TerminalLine::AddInput(int character)
{
    return AddInput(character, Asgaard::Drawing::Color(0xFF, 0xFF, 0xFF));
}

bool TerminalLine::AddInput(int character, const Asgaard::Drawing::Color& color)
{
    struct Asgaard::Drawing::Font::CharInfo bitmap = { 0 };
    
    // return false if we are full
    if (m_cursor == (int)m_cells.size()) {
        return false;
    }
    
    char buf = (char)character & 0xFF;
    if (!m_font->GetCharacterBitmap(buf, bitmap)) {
        // ignore character
        return true;
    }

    // Handle \r \t \n?
    auto& cell = m_cells[m_cursor];
    if (!cell.m_character) {
        // new cell, just add character
        m_text.push_back(character);
    }
    else {
        m_text.insert(m_cursor, &buf, 1);
        if (m_text.size() > m_cells.size()) {
            // @todo handle overflow
        }
        // shift cells right from this index
        ShiftCellsRight(m_cursor);
    }

    cell.m_character = character;
    cell.m_color = color;
    m_dirty = true;
    m_cursor++;
    return true;
}

bool TerminalLine::RemoveInput()
{
    int modifiedCursor = m_cursor - m_inputOffset;
    if (modifiedCursor != 0) {
        auto& cell = m_cells[m_cursor - 1];
        if (m_cursor != (int)m_text.length()) {
            // we were in between characters
            ShiftCellsLeft(m_cursor);
        }

        cell.m_character = 0;
        m_text.erase(m_cursor - 1, 1);
        m_dirty = true;
        m_cursor--;
        return true;
    }
    return false;
}

void TerminalLine::SetInput(const std::string& input)
{
    // clear input, then add
    if (static_cast<int>(m_text.size()) > m_inputOffset) {
        for (auto i = m_inputOffset; i < static_cast<int>(m_cells.size()); i++) {
            auto& cell = m_cells[i];
            cell.m_character = 0;
        }
        m_text.erase(m_inputOffset, std::string::npos);
        m_cursor = m_text.size();
        m_dirty = true;
    }

    auto charsToFill = std::min(input.size(), (m_cells.size() - m_inputOffset));
    for (auto i = 0u; i < charsToFill; i++) {
        AddInput(input[i]);
    }
}

void TerminalLine::Redraw(const std::shared_ptr<Asgaard::MemoryBuffer>& buffer)
{
    if (m_dirty) {
        const auto theme = Asgaard::Theming::TM.GetTheme();
        Asgaard::Drawing::Painter paint(buffer);
        int x = m_dimensions.X();
        int i = 0;

        auto drawCell = [&] (const auto& cell, auto index) {
            if (m_showCursor && m_cursor == index) {
                paint.SetFillColor(0xFF, 0xFF, 0xFF);
                paint.SetOutlineColor(0, 0, 0);
                if (!cell.m_character) {
                    paint.RenderFill(Asgaard::Rectangle(x, m_dimensions.Y(), m_font->GetFontWidth(), m_font->GetFontHeight()));
                    return;
                }
            }
            else {
                if (!cell.m_character) {
                    return;
                }

                paint.SetOutlineColor(cell.m_color);
            }
            paint.RenderCharacter(x, m_dimensions.Y(), cell.m_character);
            if (m_showCursor && m_cursor == index) {
                // restore the fill color to normal
                paint.SetFillColor(theme->GetColor(Asgaard::Theming::Theme::Colors::DEFAULT_FILL));
            }
        };
        
        paint.SetFillColor(theme->GetColor(Asgaard::Theming::Theme::Colors::DEFAULT_FILL));
        paint.RenderFill(m_dimensions);
        
        paint.SetFont(m_font);
        for (const auto& cell : m_cells) {
            drawCell(cell, i);
            x += m_font->GetFontWidth();
            i++;
        }
        m_dirty = false;
    }
}

void TerminalLine::SetCursorPosition(int position)
{
    // handle hide of cursor
    if (position == -1) {
        m_showCursor = false;
        m_dirty      = true;
        return;
    }

    // handle bounds of the cursor
    if (position < m_inputOffset) {
        return;
    }

    m_cursor     = position;
    m_showCursor = true;
    m_dirty      = true;
}

void TerminalLine::ShiftCellsRight(int index)
{
    TerminalCell overflowCell = { 0 };
    for (unsigned int i = index; i < m_cells.size(); i++) {
        auto& cell = m_cells[i];
        
        // do a swap?
        if (overflowCell.m_character) {
            TerminalCell tmpCell = { .m_character = cell.m_character, .m_color = cell.m_color };
            cell.m_character = overflowCell.m_character;
            cell.m_color = overflowCell.m_color;
            overflowCell.m_character = tmpCell.m_character;
            overflowCell.m_color = tmpCell.m_color;
        }
        else {
            overflowCell.m_character = cell.m_character;
            overflowCell.m_color = cell.m_color;
        }
    }
}

void TerminalLine::ShiftCellsLeft(int index)
{
    for (unsigned int i = (index + 1); i < m_cells.size(); i++) {
        auto& cell = m_cells[i];
        auto& previousCell = m_cells[i - 1];
        
        previousCell.m_character = cell.m_character;
        previousCell.m_color = cell.m_color;

        cell.m_character = 0;
    }
}

void TerminalLine::RebuildText()
{
    // build text from cells
    m_text.clear();
    for (const auto& cell : m_cells) {
        if (!cell.m_character) {
            break;
        } 
        m_text.push_back(cell.m_character);
    }
}

void TerminalLine::Spill(int index, int count, std::vector<TerminalCell>& storage)
{
    for (auto i = index; i < (index + count) && i < static_cast<int>(m_cells.size()); i++) {
        const auto& cell = m_cells[i];
        if (!cell.m_character) {
            break;
        }
        storage.push_back(cell);
    }
}
