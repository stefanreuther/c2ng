/**
  *  \file test/game/playerbitmatrixtest.cpp
  *  \brief Test for game::PlayerBitMatrix
  */

#include "game/playerbitmatrix.hpp"
#include "afl/test/testrunner.hpp"

/** Simple tests. */
AFL_TEST("game.PlayerBitMatrix", a)
{
    // ex GamePlayerSetTestSuite::testMatrix()
    game::PlayerBitMatrix mtx;

    // check MAX_PLAYERS. Must fix some tests below if this changes.
    using game::MAX_PLAYERS;
    a.checkLessThan("01", MAX_PLAYERS, 90);

    // check zero-initialisation
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        for (int j = 1; j <= MAX_PLAYERS; ++j) {
            a.check("11", !mtx.get(i, j));
        }
    }
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        a.check("12", mtx.getRow(i).empty());
    }

    // check out-of-range access
    a.check("21", mtx.getRow(-1).empty());
    a.check("22", mtx.getRow(10000).empty());

    // set some value and check that surroundings are not modified. Do so twice.
    for (int i = 0; i < 2; ++i) {
        mtx.set(3, 7, true);
        a.check("31", !mtx.get(2, 6));
        a.check("32", !mtx.get(2, 7));
        a.check("33", !mtx.get(2, 8));
        a.check("34", !mtx.get(3, 6));
        a.check("35",  mtx.get(3, 7));
        a.check("36", !mtx.get(3, 8));
        a.check("37", !mtx.get(4, 6));
        a.check("38", !mtx.get(4, 7));
        a.check("39", !mtx.get(4, 8));
        a.check("40", mtx.getRow(2).empty());
        a.checkEqual("41", mtx.getRow(3), game::PlayerSet_t(7));
        a.check("42", mtx.getRow(4).empty());
    }

    // set neighbouring bit (same row)
    for (int i = 0; i < 2; ++i) {
        mtx.set(3, 8, true);
        a.check("51", !mtx.get(2, 7));
        a.check("52", !mtx.get(2, 8));
        a.check("53", !mtx.get(2, 9));
        a.check("54",  mtx.get(3, 7));
        a.check("55",  mtx.get(3, 8));
        a.check("56", !mtx.get(3, 9));
        a.check("57", !mtx.get(4, 7));
        a.check("58", !mtx.get(4, 8));
        a.check("59", !mtx.get(4, 9));
        a.check("60", mtx.getRow(2).empty());
        a.checkEqual("61", mtx.getRow(3), game::PlayerSet_t(7) | game::PlayerSet_t(8));
        a.check("62", mtx.getRow(4).empty());
    }

    // set neighbouring bit (different row)
    for (int i = 0; i < 2; ++i) {
        mtx.set(2, 6, true);
        a.check("71", !mtx.get(1, 5));
        a.check("72", !mtx.get(1, 6));
        a.check("73", !mtx.get(1, 7));
        a.check("74", !mtx.get(2, 5));
        a.check("75",  mtx.get(2, 6));
        a.check("76", !mtx.get(2, 7));
        a.check("77", !mtx.get(3, 5));
        a.check("78", !mtx.get(3, 6));
        a.check("79",  mtx.get(3, 7));
        a.check("80", mtx.getRow(1).empty());
        a.checkEqual("81", mtx.getRow(2), game::PlayerSet_t(6));
        a.checkEqual("82", mtx.getRow(3), game::PlayerSet_t(7) | game::PlayerSet_t(8));
    }

    // clear bit
    for (int i = 0; i < 2; ++i) {
        mtx.set(2, 6, false);
        a.check("91", !mtx.get(2, 6));
        a.check("92", mtx.getRow(2).empty());
    }

    // check some out-of-range positions
    a.check("101", !mtx.get(99, 2));
    a.check("102", !mtx.get(99, 99));
    a.check("103", !mtx.get(2, 99));
    a.check("104", !mtx.get(-99, 99));
    a.check("105", !mtx.get(1, 130));
    a.check("106", !mtx.get(130, 1));

    // out-of-range
    mtx.set(0, 1, true);
    a.check("111", !mtx.get(0, 1));
    mtx.set(1, 0, true);
    a.check("112", !mtx.get(0, 1));

    mtx.set(1000, 1, true);
    a.check("121", !mtx.get(1000, 1));
    mtx.set(1, 1000, true);
    a.check("122", !mtx.get(1000, 1));

    // clear it again and check zeroness again
    mtx.clear();

    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        for (int j = 1; j <= MAX_PLAYERS; ++j) {
            a.check("131", !mtx.get(i, j));
        }
    }
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        a.check("132", mtx.getRow(i).empty());
    }
}
