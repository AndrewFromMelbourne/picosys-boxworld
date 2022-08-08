//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2022 Andrew Duncan
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------

#include "picosystem.hpp"

#include "boxworld.h"
#include "font.h"
#include "images.h"

//-------------------------------------------------------------------------

namespace ps = picosystem;

//-------------------------------------------------------------------------

Boxworld::Boxworld()
:
    m_level{0},
    m_levelSolved{false},
    m_canUndo{false},
    m_player{ 0, 0 },
    m_board(),
    m_boardPrevious(),
    m_levels(),
    m_tileBuffers(
        { {
            { tileWidth, tileHeight, emptyImage },
            { tileWidth, tileHeight, passageImage },
            { tileWidth, tileHeight, boxImage },
            { tileWidth * 2, tileHeight, playerImage },
            { tileWidth, tileHeight, wallImage },
            { tileWidth, tileHeight, passageWithTargetImage },
            { tileWidth, tileHeight, boxOnTargetImage },
            { tileWidth * 2, tileHeight, playerOnTargetImage }
        } }),
    m_batteryBuffer{ batteryWidth, batteryHeight, batteryImage }
{
}

//-------------------------------------------------------------------------

void
Boxworld::init()
{
    ps::font(fontWidth, fontWidth, fontSpacingPixels, thin_font[0]);
    m_levelSolved = false;
    m_board = m_levels.level(m_level);
    m_boardPrevious = m_board;
    m_canUndo = false;
    findPlayer();
}

//-------------------------------------------------------------------------

void
Boxworld::update(uint32_t tick)
{
    if (ps::pressed(ps::A))
    {
        if (m_level < (Level::levelCount - 1))
        {
            ++m_level;
            init();
        }
    }
    else if (ps::pressed(ps::B))
    {
        if (m_level > 0)
        {
            --m_level;
            init();
        }
    }
    else if (ps::pressed(ps::X))
    {
        if (m_canUndo)
        {
            m_board = m_boardPrevious;
            findPlayer();
            m_canUndo = false;
        }
    }
    else if (ps::pressed(ps::Y))
    {
        init();
    }
    else
    {
        int dx = 0;
        int dy = 0;

        if (ps::pressed(ps::UP))
        {
            dy = -1;
        }
        else if (ps::pressed(ps::DOWN))
        {
            dy = 1;
        }
        else if (ps::pressed(ps::LEFT))
        {
            dx = -1;
        }
        else if (ps::pressed(ps::RIGHT))
        {
            dx = 1;
        }

        if ((dx != 0) or (dy != 0))
        {
            Location next{ .x = m_player.x + dx, .y = m_player.y + dy };
            auto piece1 = m_board[next.y][next.x] & ~targetMask;

            if (piece1 == PASSAGE)
            {
                swapPieces(m_player, next);
                m_player = next;
            }
            else if (piece1 == BOX)
            {
                Location afterBox{ .x = next.x + dx, .y = next.y + dy };
                auto piece2 = m_board[afterBox.y][afterBox.x] & ~targetMask;

                if (piece2 == PASSAGE)
                {
                    m_boardPrevious =  m_board;
                    swapPieces(next, afterBox);
                    swapPieces(m_player, next);
                    m_player = next;

                    isLevelSolved();
                    m_canUndo = not m_levelSolved;
                }
            }
        }
    }
}

//-------------------------------------------------------------------------

void
Boxworld::draw(uint32_t tick)
{
    ps::pen(1, 3, 5);
    ps::clear();

    drawBoard(tick);
    drawText(tick);
    drawBattery(ps::SCREEN->w - m_batteryBuffer.w - 1, 1);
}

//-------------------------------------------------------------------------

void
Boxworld::drawBoard(uint32_t tick)
{
    for (int j = 0 ; j < Level::levelHeight ; ++j)
    {
        for (int i = 0 ; i < Level::levelWidth ; ++i)
        {
            auto piece = m_board[j][i];
            auto tileBuffer = &(m_tileBuffers[piece]);
            int startX = 0;

            if (tileBuffer->w > tileWidth)
            {
                auto frames = tileBuffer->w / tileWidth;
                auto frame = (tick / 10) % frames;

                startX = frame *  tileWidth;
            }

            ps::blit(tileBuffer,
                     startX,
                     0,
                     tileWidth,
                     tileHeight,
                     i * tileWidth,
                     (j * tileHeight) + boardYoffset);
        }
    }
}

//-------------------------------------------------------------------------

void
Boxworld::drawText(uint32_t tick)
{
    const int halfScreenWidth = ps::SCREEN->w / 2;
    constexpr int row1 = boardYend + 2;
    constexpr int row2 = row1 + fontHeight + 2;

    const std::string penText = "\\penFFFF";
    const std::string penBold = "\\penFF0F";
    const std::string penDisabled = "\\penAAFA";
    const std::string penUndo = ((m_canUndo) ? penText : penDisabled);
    const std::string penNextLevel = (m_level < (Level::levelCount - 1)) ? penText : penDisabled;
    const std::string penPreviousLevel = (m_level > 0) ? penText : penDisabled;

    std::string labelLevel = penBold + "level: " + penText + std::to_string(m_level + 1);

    if (m_levelSolved)
    {
        const std::string penSolved = "\\penF0FF";
        const std::string labelLevelSolved = penSolved + " [solved]";
        labelLevel += labelLevelSolved;
    }

    const std::string labelA = penBold + "(A): " + penNextLevel + "next level";
    const std::string labelB = penBold + "(B): " + penPreviousLevel + "previous level";

    const std::string labelX = penBold + "(X): " + penUndo + "undo box move";
    const std::string labelY = penBold + "(Y): " + penText + "restart level";

    ps::text(labelLevel, 1, 1);

    ps::text(labelX, 1, row1);
    ps::text(labelY, 1, row2);

    ps::text(labelA, halfScreenWidth, row1);
    ps::text(labelB, halfScreenWidth, row2);
}

//-------------------------------------------------------------------------

void
Boxworld::drawBattery(int x, int y)
{
    int level = ps::battery() / 10;
    ps::color_t levelColour = (level > 1) ? 0x0ff0 : 0x00ff;
    ps::color_t backgroundColour = 0x00f0;

    for (int i = 0 ; i < 10 ; ++i)
    {
        ps::color_t colour = (level > i) ? levelColour : backgroundColour;

        for (int j = 0 ; j < 6 ; ++j)
        {
            m_batteryBuffer.data[(i + 1) + ((j + 1) * m_batteryBuffer.w)] = colour;
        }
    }

    ps::blit(&m_batteryBuffer, 0, 0, m_batteryBuffer.w, m_batteryBuffer.h, x, y);
}

//-------------------------------------------------------------------------

void
Boxworld::findPlayer()
{
    bool found = false;

    for (int j = 0 ; (j < Level::levelHeight) and not found ; ++j)
    {
        for (int i = 0 ; (i < Level::levelWidth) and not found ; ++i)
        {
            if ((m_board[j][i] & ~targetMask) == PLAYER)
            {
                found = true;
                m_player.x = i;
                m_player.y = j;
            }
        }
    }
}

//-------------------------------------------------------------------------

void
Boxworld::swapPieces(const Location& location1, const Location& location2)
{
    auto piece1 = m_board[location1.y][location1.x] & ~targetMask;
    auto piece2 = m_board[location2.y][location2.x] & ~targetMask;

    m_board[location1.y][location1.x] = (m_board[location1.y][location1.x] & targetMask) | piece2;
    m_board[location2.y][location2.x] = (m_board[location2.y][location2.x] & targetMask) | piece1;
}

//-------------------------------------------------------------------------

void
Boxworld::isLevelSolved()
{
    m_levelSolved = true;

    for (int j = 0 ; (j < Level::levelHeight) and m_levelSolved ; ++j)
    {
        for (int i = 0 ; (i < Level::levelWidth) and m_levelSolved ; ++i)
        {
            if (m_board[j][i] == BOX)
            {
                m_levelSolved = false;
            }
        }
    }
}

