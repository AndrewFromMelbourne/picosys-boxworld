#pragma once

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

#include "images.h"
#include "level.h"
#include "levels.h"

//-------------------------------------------------------------------------

class Boxworld
{
public:

    struct Location
    {
        int x;
        int y;
    };

    //---------------------------------------------------------------------

    enum Pieces
    {
        EMPTY = 0x00,
        PASSAGE = 0x01,
        BOX = 0x02,
        PLAYER = 0x03,
        WALL = 0x04,
        PASSAGE_WITH_TARGET = 0x05,
        BOX_ON_TARGET = 0x06,
        PLAYER_ON_TARGER = 0x07
    };

    //---------------------------------------------------------------------

    static constexpr uint8_t targetMask = 0x04;

    static constexpr int boardYoffset = 10;
    static constexpr int boardYend = boardYoffset + (tileHeight * Level::levelHeight);

    //---------------------------------------------------------------------

    Boxworld();

    void init();
    void update(uint32_t tick);
    void draw(uint32_t tick);

private:

    void findPlayer();
    void swapPieces(const Location& location1, const Location& location2);
    void isLevelSolved();
    void drawBoard(uint32_t tick);
    void drawText(uint32_t tick);
    void drawBattery(int x, int y);

    //---------------------------------------------------------------------

    int m_level;
    bool m_levelSolved;
    bool m_canUndo;
    Location m_player;
    Level::LevelType m_board;
    Level::LevelType m_boardPrevious;
    const Levels m_levels;
    std::array<picosystem::buffer_t, tileCount> m_tileBuffers;
    picosystem::buffer_t m_batteryBuffer;
};

