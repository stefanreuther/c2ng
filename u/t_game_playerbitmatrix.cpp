/**
  *  \file u/t_game_playerbitmatrix.cpp
  *  \brief Test for game::PlayerBitMatrix
  */

#include "game/playerbitmatrix.hpp"

#include "t_game.hpp"

void
TestGamePlayerBitMatrix::testMatrix()
{
    // ex GamePlayerSetTestSuite::testMatrix()
    game::PlayerBitMatrix mtx;

    // check MAX_PLAYERS. Must fix some tests below if this changes.
    using game::MAX_PLAYERS;
    TS_ASSERT(MAX_PLAYERS < 90);

    // check zero-initialisation
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        for (int j = 1; j <= MAX_PLAYERS; ++j) {
            TS_ASSERT(!mtx.get(i, j));
        }
    }
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        TS_ASSERT(mtx.getRow(i).empty());
    }

    // set some value and check that surroundings are not modified. Do so twice.
    for (int i = 0; i < 2; ++i) {
        mtx.set(3, 7, true);
        TS_ASSERT(!mtx.get(2, 6));
        TS_ASSERT(!mtx.get(2, 7));
        TS_ASSERT(!mtx.get(2, 8));
        TS_ASSERT(!mtx.get(3, 6));
        TS_ASSERT( mtx.get(3, 7));
        TS_ASSERT(!mtx.get(3, 8));
        TS_ASSERT(!mtx.get(4, 6));
        TS_ASSERT(!mtx.get(4, 7));
        TS_ASSERT(!mtx.get(4, 8));
        TS_ASSERT(mtx.getRow(2).empty());
        TS_ASSERT_EQUALS(mtx.getRow(3), game::PlayerSet_t(7));
        TS_ASSERT(mtx.getRow(4).empty());
    }

    // set neighbouring bit (same row)
    for (int i = 0; i < 2; ++i) {
        mtx.set(3, 8, true);
        TS_ASSERT(!mtx.get(2, 7));
        TS_ASSERT(!mtx.get(2, 8));
        TS_ASSERT(!mtx.get(2, 9));
        TS_ASSERT( mtx.get(3, 7));
        TS_ASSERT( mtx.get(3, 8));
        TS_ASSERT(!mtx.get(3, 9));
        TS_ASSERT(!mtx.get(4, 7));
        TS_ASSERT(!mtx.get(4, 8));
        TS_ASSERT(!mtx.get(4, 9));
        TS_ASSERT(mtx.getRow(2).empty());
        TS_ASSERT_EQUALS(mtx.getRow(3), game::PlayerSet_t(7) | game::PlayerSet_t(8));
        TS_ASSERT(mtx.getRow(4).empty());
    }

    // set neighbouring bit (different row)
    for (int i = 0; i < 2; ++i) {
        mtx.set(2, 6, true);
        TS_ASSERT(!mtx.get(1, 5));
        TS_ASSERT(!mtx.get(1, 6));
        TS_ASSERT(!mtx.get(1, 7));
        TS_ASSERT(!mtx.get(2, 5));
        TS_ASSERT( mtx.get(2, 6));
        TS_ASSERT(!mtx.get(2, 7));
        TS_ASSERT(!mtx.get(3, 5));
        TS_ASSERT(!mtx.get(3, 6));
        TS_ASSERT( mtx.get(3, 7));
        TS_ASSERT(mtx.getRow(1).empty());
        TS_ASSERT_EQUALS(mtx.getRow(2), game::PlayerSet_t(6));
        TS_ASSERT_EQUALS(mtx.getRow(3), game::PlayerSet_t(7) | game::PlayerSet_t(8));
    }

    // check some out-of-range positions
    TS_ASSERT(!mtx.get(99, 2));
    TS_ASSERT(!mtx.get(99, 99));
    TS_ASSERT(!mtx.get(2, 99));
    TS_ASSERT(!mtx.get(-99, 99));
    TS_ASSERT(!mtx.get(1, 130));
    TS_ASSERT(!mtx.get(130, 1));

    // clear it again and check zeroness again
    mtx.clear();

    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        for (int j = 1; j <= MAX_PLAYERS; ++j) {
            TS_ASSERT(!mtx.get(i, j));
        }
    }
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        TS_ASSERT(mtx.getRow(i).empty());
    }
}
