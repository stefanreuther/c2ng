/**
  *  \file u/t_game_vcr_nulldatabase.cpp
  *  \brief Test for game::vcr::NullDatabase
  */

#include "game/vcr/nulldatabase.hpp"

#include "t_game_vcr.hpp"

void
TestGameVcrNullDatabase::testIt()
{
    game::vcr::NullDatabase testee;
    TS_ASSERT_EQUALS(testee.getNumBattles(), 0U);
    TS_ASSERT(testee.getBattle(0) == 0);
    TS_ASSERT(testee.getBattle(10000) == 0);
}

