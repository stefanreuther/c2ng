/**
  *  \file test/server/host/rank/ranktest.cpp
  *  \brief Test for server::host::rank::Rank
  */

#include "server/host/rank/rank.hpp"
#include "afl/test/testrunner.hpp"

using server::host::rank::Rank_t;
using server::host::rank::initRanks;
using server::host::rank::compactRanks;
using game::PlayerSet_t;
using server::host::Game;

/** Plain score ranking. */
AFL_TEST("server.host.rank.Rank:compactRanks:normal", a)
{
    // Scores          #2    #4    #3    #5    #4    #1    #7    #6    #8    #7    #9
    Rank_t scores = { -500, -400, -450, -300, -400, -600, -200, -250, -100, -200, -50 };
    Rank_t null; initRanks(null, 0x7FFFFFFF);
    Rank_t result;
    compactRanks(result, scores, null, PlayerSet_t::allUpTo(Game::NUM_PLAYERS));
    a.checkEqual("01", result[0],  2);
    a.checkEqual("02", result[1],  4);
    a.checkEqual("03", result[2],  3);
    a.checkEqual("04", result[3],  5);
    a.checkEqual("05", result[4],  4);
    a.checkEqual("06", result[5],  1);
    a.checkEqual("07", result[6],  7);
    a.checkEqual("08", result[7],  6);
    a.checkEqual("09", result[8],  8);
    a.checkEqual("10", result[9],  7);
    a.checkEqual("11", result[10], 9);
}

/** Plain score ranking, with highest scores not playing. */
AFL_TEST("server.host.rank.Rank:compactRanks:not-playing", a)
{
    // Scores            -   #2    #1    #3    #2     -    #5    #4    #6    #5    #7
    Rank_t scores = { -500, -400, -450, -300, -400, -600, -200, -250, -100, -200, -50 };
    Rank_t null; initRanks(null, 0x7FFFFFFF);
    Rank_t result;
    compactRanks(result, scores, null, PlayerSet_t::allUpTo(Game::NUM_PLAYERS) - 6 - 1);
    a.checkEqual("01", result[1],  2);
    a.checkEqual("02", result[2],  1);
    a.checkEqual("03", result[3],  3);
    a.checkEqual("04", result[4],  2);
    a.checkEqual("05", result[6],  5);
    a.checkEqual("06", result[7],  4);
    a.checkEqual("07", result[8],  6);
    a.checkEqual("08", result[9],  5);
    a.checkEqual("09", result[10], 7);
}

/** Turns-over-limit scoring. */
AFL_TEST("server.host.rank.Rank:compactRanks:turns-over-limit", a)
{
    // Scores         #4   #5    #3   #2  #5   #6   #7   #5    #1   #8   #1
    Rank_t turns  = { -1,  0,    -2,  -3,   0,   0,   0,   0,  -5,   0,  -5 };
    Rank_t scores = { 500, 400, 500, 500, 400, 300, 200, 400, 500, 100, 500 };
    Rank_t result;
    compactRanks(result, turns, scores, PlayerSet_t::allUpTo(Game::NUM_PLAYERS));
    a.checkEqual("01", result[0],  4);
    a.checkEqual("02", result[1],  5);
    a.checkEqual("03", result[2],  3);
    a.checkEqual("04", result[3],  2);
    a.checkEqual("05", result[4],  5);
    a.checkEqual("06", result[5],  6);
    a.checkEqual("07", result[6],  7);
    a.checkEqual("08", result[7],  5);
    a.checkEqual("09", result[8],  1);
    a.checkEqual("10", result[9],  8);
    a.checkEqual("11", result[10], 1);
}

/** Turns-over-limit scoring. */
AFL_TEST("server.host.rank.Rank:compactRanks:turns-over-limit:2", a)
{
    // Scores         #5   #6    #4   #3  #6   #7   #8   #6    #1   #9   #2
    Rank_t turns  = { -1,  0,    -2,  -3,   0,   0,   0,   0,  -5,   0,  -5 };
    Rank_t scores = { 500, 400, 500, 500, 400, 300, 200, 400, 501, 100, 500 };
    Rank_t result;
    compactRanks(result, turns, scores, PlayerSet_t::allUpTo(Game::NUM_PLAYERS));
    a.checkEqual("01", result[0],  5);
    a.checkEqual("02", result[1],  6);
    a.checkEqual("03", result[2],  4);
    a.checkEqual("04", result[3],  3);
    a.checkEqual("05", result[4],  6);
    a.checkEqual("06", result[5],  7);
    a.checkEqual("07", result[6],  8);
    a.checkEqual("08", result[7],  6);
    a.checkEqual("09", result[8],  1);
    a.checkEqual("10", result[9],  9);
    a.checkEqual("11", result[10], 2);
}

/** Ranking with a partial c2ref.txt file. */
AFL_TEST("server.host.rank.Rank:compactRanks:partial-data", a)
{
    Rank_t ranks; initRanks(ranks, 0x7FFFFFFF);
    ranks[5] = 1;
    ranks[3] = 5;
    ranks[9] = 10;
    Rank_t scores; initRanks(scores, 0x7FFFFFFF);
    Rank_t result;
    compactRanks(result, ranks, scores, PlayerSet_t::allUpTo(Game::NUM_PLAYERS));
    a.checkEqual("01", result[0],  4);
    a.checkEqual("02", result[1],  4);
    a.checkEqual("03", result[2],  4);
    a.checkEqual("04", result[3],  2);
    a.checkEqual("05", result[4],  4);
    a.checkEqual("06", result[5],  1);
    a.checkEqual("07", result[6],  4);
    a.checkEqual("08", result[7],  4);
    a.checkEqual("09", result[8],  4);
    a.checkEqual("10", result[9],  3);
    a.checkEqual("11", result[10], 4);
}

/** Ranking with a partial c2ref.txt file and not all players participating.
    This is missing player 4 (ranks[3]). */
AFL_TEST("server.host.rank.Rank:compactRanks:partial-data:2", a)
{
    Rank_t ranks; initRanks(ranks, 0x7FFFFFFF);
    ranks[5] = 1;
    ranks[3] = 5;
    ranks[9] = 10;
    Rank_t scores; initRanks(scores, 0x7FFFFFFF);
    Rank_t result;
    compactRanks(result, ranks, scores, PlayerSet_t::allUpTo(Game::NUM_PLAYERS) - 4);
    a.checkEqual("01", result[0],  3);
    a.checkEqual("02", result[1],  3);
    a.checkEqual("03", result[2],  3);
    a.checkEqual("04", result[4],  3);
    a.checkEqual("05", result[5],  1);
    a.checkEqual("06", result[6],  3);
    a.checkEqual("07", result[7],  3);
    a.checkEqual("08", result[8],  3);
    a.checkEqual("09", result[9],  2);
    a.checkEqual("10", result[10], 3);
}
