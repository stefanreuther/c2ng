/**
  *  \file u/t_server_host_rank_rank.cpp
  *  \brief Test for server::host::rank::Rank
  */

#include "server/host/rank/rank.hpp"

#include "t_server_host_rank.hpp"

using server::host::rank::Rank_t;
using server::host::rank::initRanks;
using server::host::rank::compactRanks;
using game::PlayerSet_t;
using server::host::Game;

/** Plain score ranking. */
void
TestServerHostRankRank::testSimple()
{
    // Scores          #2    #4    #3    #5    #4    #1    #7    #6    #8    #7    #9
    Rank_t scores = { -500, -400, -450, -300, -400, -600, -200, -250, -100, -200, -50 };
    Rank_t null; initRanks(null, 0x7FFFFFFF);
    Rank_t result;
    compactRanks(result, scores, null, PlayerSet_t::allUpTo(Game::NUM_PLAYERS));
    TS_ASSERT_EQUALS(result[0],  2);
    TS_ASSERT_EQUALS(result[1],  4);
    TS_ASSERT_EQUALS(result[2],  3);
    TS_ASSERT_EQUALS(result[3],  5);
    TS_ASSERT_EQUALS(result[4],  4);
    TS_ASSERT_EQUALS(result[5],  1);
    TS_ASSERT_EQUALS(result[6],  7);
    TS_ASSERT_EQUALS(result[7],  6);
    TS_ASSERT_EQUALS(result[8],  8);
    TS_ASSERT_EQUALS(result[9],  7);
    TS_ASSERT_EQUALS(result[10], 9);
}

/** Plain score ranking, with highest scores not playing. */
void
TestServerHostRankRank::testNotPlaying()
{
    // Scores            -   #2    #1    #3    #2     -    #5    #4    #6    #5    #7
    Rank_t scores = { -500, -400, -450, -300, -400, -600, -200, -250, -100, -200, -50 };
    Rank_t null; initRanks(null, 0x7FFFFFFF);
    Rank_t result;
    compactRanks(result, scores, null, PlayerSet_t::allUpTo(Game::NUM_PLAYERS) - 6 - 1);
    TS_ASSERT_EQUALS(result[1],  2);
    TS_ASSERT_EQUALS(result[2],  1);
    TS_ASSERT_EQUALS(result[3],  3);
    TS_ASSERT_EQUALS(result[4],  2);
    TS_ASSERT_EQUALS(result[6],  5);
    TS_ASSERT_EQUALS(result[7],  4);
    TS_ASSERT_EQUALS(result[8],  6);
    TS_ASSERT_EQUALS(result[9],  5);
    TS_ASSERT_EQUALS(result[10], 7);
}

/** Turns-over-limit scoring. */
void
TestServerHostRankRank::testTurnOverLimit()
{
    // Scores         #4   #5    #3   #2  #5   #6   #7   #5    #1   #8   #1
    Rank_t turns  = { -1,  0,    -2,  -3,   0,   0,   0,   0,  -5,   0,  -5 };
    Rank_t scores = { 500, 400, 500, 500, 400, 300, 200, 400, 500, 100, 500 };
    Rank_t result;
    compactRanks(result, turns, scores, PlayerSet_t::allUpTo(Game::NUM_PLAYERS));
    TS_ASSERT_EQUALS(result[0],  4);
    TS_ASSERT_EQUALS(result[1],  5);
    TS_ASSERT_EQUALS(result[2],  3);
    TS_ASSERT_EQUALS(result[3],  2);
    TS_ASSERT_EQUALS(result[4],  5);
    TS_ASSERT_EQUALS(result[5],  6);
    TS_ASSERT_EQUALS(result[6],  7);
    TS_ASSERT_EQUALS(result[7],  5);
    TS_ASSERT_EQUALS(result[8],  1);
    TS_ASSERT_EQUALS(result[9],  8);
    TS_ASSERT_EQUALS(result[10], 1);
}

/** Turns-over-limit scoring. */
void
TestServerHostRankRank::testTurnOverLimit2()
{
    // Scores         #5   #6    #4   #3  #6   #7   #8   #6    #1   #9   #2
    Rank_t turns  = { -1,  0,    -2,  -3,   0,   0,   0,   0,  -5,   0,  -5 };
    Rank_t scores = { 500, 400, 500, 500, 400, 300, 200, 400, 501, 100, 500 };
    Rank_t result;
    compactRanks(result, turns, scores, PlayerSet_t::allUpTo(Game::NUM_PLAYERS));
    TS_ASSERT_EQUALS(result[0],  5);
    TS_ASSERT_EQUALS(result[1],  6);
    TS_ASSERT_EQUALS(result[2],  4);
    TS_ASSERT_EQUALS(result[3],  3);
    TS_ASSERT_EQUALS(result[4],  6);
    TS_ASSERT_EQUALS(result[5],  7);
    TS_ASSERT_EQUALS(result[6],  8);
    TS_ASSERT_EQUALS(result[7],  6);
    TS_ASSERT_EQUALS(result[8],  1);
    TS_ASSERT_EQUALS(result[9],  9);
    TS_ASSERT_EQUALS(result[10], 2);
}

/** Ranking with a partial c2ref.txt file. */
void
TestServerHostRankRank::testPartial()
{
    Rank_t ranks; initRanks(ranks, 0x7FFFFFFF);
    ranks[5] = 1;
    ranks[3] = 5;
    ranks[9] = 10;
    Rank_t scores; initRanks(scores, 0x7FFFFFFF);
    Rank_t result;
    compactRanks(result, ranks, scores, PlayerSet_t::allUpTo(Game::NUM_PLAYERS));
    TS_ASSERT_EQUALS(result[0],  4);
    TS_ASSERT_EQUALS(result[1],  4);
    TS_ASSERT_EQUALS(result[2],  4);
    TS_ASSERT_EQUALS(result[3],  2);
    TS_ASSERT_EQUALS(result[4],  4);
    TS_ASSERT_EQUALS(result[5],  1);
    TS_ASSERT_EQUALS(result[6],  4);
    TS_ASSERT_EQUALS(result[7],  4);
    TS_ASSERT_EQUALS(result[8],  4);
    TS_ASSERT_EQUALS(result[9],  3);
    TS_ASSERT_EQUALS(result[10], 4);
}

/** Ranking with a partial c2ref.txt file and not all players participating.
    This is missing player 4 (ranks[3]). */
void
TestServerHostRankRank::testPartial2()
{
    Rank_t ranks; initRanks(ranks, 0x7FFFFFFF);
    ranks[5] = 1;
    ranks[3] = 5;
    ranks[9] = 10;
    Rank_t scores; initRanks(scores, 0x7FFFFFFF);
    Rank_t result;
    compactRanks(result, ranks, scores, PlayerSet_t::allUpTo(Game::NUM_PLAYERS) - 4);
    TS_ASSERT_EQUALS(result[0],  3);
    TS_ASSERT_EQUALS(result[1],  3);
    TS_ASSERT_EQUALS(result[2],  3);
    TS_ASSERT_EQUALS(result[4],  3);
    TS_ASSERT_EQUALS(result[5],  1);
    TS_ASSERT_EQUALS(result[6],  3);
    TS_ASSERT_EQUALS(result[7],  3);
    TS_ASSERT_EQUALS(result[8],  3);
    TS_ASSERT_EQUALS(result[9],  2);
    TS_ASSERT_EQUALS(result[10], 3);
}
