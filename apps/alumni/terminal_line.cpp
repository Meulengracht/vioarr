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

TerminalLine::TerminalLine(const std::shared_ptr<Asgaard::Drawing::Font>& font, int row, int initialCellCount)
    : m_font(font)
    , m_dimensions(ALUMNI_MARGIN_LEFT, (
                   row * font->GetFontHeight()) + ALUMNI_MARGIN_TOP, 
                   initialCellCount * font->GetFontWidth(), 
                   font->GetFontHeight())
    , m_row(row)
    , m_cells(initialCellCount)
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

    // build text from cells
    m_text.clear();
    for (const auto& cell : m_cells) {
        if (!cell.m_character) {
            break;
        } 
        m_text.push_back(cell.m_character);
    }
}

void TerminalLine::Resize(int cellCount)
{
    if (cellCount < m_cells.size()) {
        // shrinking, means we will spill cells
        // @todo
    }
    else {
        // growing, just extend our cell count
        m_cells.resize(cellCount);
    }
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
    if (m_cursor == m_cells.size()) {
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
        if (m_cursor != m_text.length()) {
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

void TerminalLine::Redraw(std::shared_ptr<Asgaard::MemoryBuffer>& buffer)
{
    if (m_dirty) {
        Asgaard::Drawing::Painter paint(buffer);
        int x = m_dimensions.X();
        int i = 0;
        
        paint.SetFillColor(0x7F, 0x0C, 0x35, 0x33);
        paint.RenderFill(m_dimensions);
        
        paint.SetFont(m_font);
        for (const auto& cell : m_cells) {
            if (!cell.m_character) {
                break;
            }

            paint.SetOutlineColor(cell.m_color);
            paint.RenderCharacter(x, m_dimensions.Y(), cell.m_character);

            x += m_font->GetFontWidth();
            i++;
        }
        
        if (m_showCursor) {
            paint.SetFillColor(0xFF, 0xFF, 0xFF);
            paint.RenderFill(Asgaard::Rectangle(x, m_dimensions.Y(), m_font->GetFontWidth(), m_font->GetFontHeight()));
        }
        m_dirty = false;
    }
}

void TerminalLine::HideCursor()
{
    m_showCursor = false;
    m_dirty      = true;
}

void TerminalLine::ShowCursor()
{
    m_showCursor = true;
    m_dirty      = true;
}

void TerminalLine::ShiftCellsRight(int index)
{
    TerminalCell overflowCell = { 0 };
    for (int i = index; i < m_cells.size(); i++) {
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
    for (int i = (index + 1); i < m_cells.size(); i++) {
        auto& cell = m_cells[i];
        auto& previousCell = m_cells[i - 1];
        
        previousCell.m_character = cell.m_character;
        previousCell.m_color = cell.m_color;

        cell.m_character = 0;
    }
}
