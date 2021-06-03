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

#include <drawing/font.hpp>
#include <drawing/painter.hpp>
#include <drawing/color.hpp>
#include <memory>
#include <vector>
#include <string>

struct TerminalCell {
    int                     m_character;
    Asgaard::Drawing::Color m_color;
};

class TerminalLine {
public:
    TerminalLine(const std::shared_ptr<Asgaard::Drawing::Font>&, int row, int initialCellCount);
    ~TerminalLine() = default;

    void Reset();
    void Reset(const std::vector<TerminalCell>&);
    void Resize(int cellCount, std::vector<TerminalCell>& overflownCells);
    void PrependCells(const std::vector<TerminalCell>& cells, std::vector<TerminalCell>& overflownCells);
    int  AppendCells(const std::vector<TerminalCell>& cells);
    void Redraw(std::shared_ptr<Asgaard::MemoryBuffer>&);

    /**
     * @brief Adds user input to the line. The difference here is that input will move
     * the cursor position as well.
     * 
     * @param character The character to print
     * @return true  If the input could fit into the line
     * @return false If the input did not fit, the input is at end of line
     */
    bool AddInput(int character);
    bool AddInput(int character, const Asgaard::Drawing::Color& color);
    bool RemoveInput();
    void SetInput(const std::string& input);

    /**
     * @brief Adds non-input text to the line, this text can not be changed by
     * the cursor. 
     * 
     * @param character The character to print
     * @return true  If the input could fit into the line
     * @return false If the input did not fit, the input is at end of line
     */
    bool AddCharacter(int character);
    bool AddCharacter(int character, const Asgaard::Drawing::Color& color);

    /**
     * @brief Sets the cursor input position, new input will be written to this location
     * instead of at end of line. 
     * 
     * @param position -1 to hide the cursor, otherwise it's the index of the line
     */
    void SetCursorPosition(int position = -1);

    int GetCursorPosition() const { return m_cursor; }
    int GetCurrentLength()  const { return m_text.size(); }
    const std::vector<TerminalCell>& GetCells() const { return m_cells; }

private:
    void ShiftCellsRight(int index);
    void ShiftCellsLeft(int index);
    void RebuildText();
    void Spill(int index, int count, std::vector<TerminalCell>&);

private:
    std::shared_ptr<Asgaard::Drawing::Font> m_font;
    
    Asgaard::Rectangle        m_dimensions;
    std::vector<TerminalCell> m_cells;
    std::string               m_text;
    int                       m_row;
    int                       m_cursor;
    int                       m_inputOffset;
    bool                      m_showCursor;
    bool                      m_dirty;
};

class TerminalLineHistory {
public:
    TerminalLineHistory(const std::unique_ptr<TerminalLine>& line)
        : m_cells(line->GetCells()) { }

    const std::vector<TerminalCell>& GetCells() const { return m_cells; } 

private:
    std::vector<TerminalCell> m_cells;
};
