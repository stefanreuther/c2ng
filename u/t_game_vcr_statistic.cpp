/**
  *  \file u/t_game_vcr_statistic.cpp
  *  \brief Test for game::vcr::Statistic
  */

#include "game/vcr/statistic.hpp"

#include "t_game_vcr.hpp"
#include "game/vcr/object.hpp"

/** Test Statistic operations.
    A: create a Statistic object
    E: "inquiry" calls report empty content. */
void
TestGameVcrStatistic::testInit()
{
    game::vcr::Statistic t;
    TS_ASSERT_EQUALS(t.getMinFightersAboard(), 0);
    TS_ASSERT_EQUALS(t.getNumTorpedoHits(), 0);
    TS_ASSERT_EQUALS(t.getNumFights(), 0);
}

/** Test Statistic operations.
    A: execute a sequence of "record" calls.
    E: "inquiry" calls produce expected results. */
void
TestGameVcrStatistic::testIt()
{
    game::vcr::Object obj;
    obj.setNumFighters(30);

    // Initialize
    game::vcr::Statistic t;
    t.init(obj, 1);
    TS_ASSERT_EQUALS(t.getMinFightersAboard(), 30);
    TS_ASSERT_EQUALS(t.getNumTorpedoHits(), 0);
    TS_ASSERT_EQUALS(t.getNumFights(), 1);

    // Some action
    t.handleFightersAboard(20);
    t.handleFightersAboard(25);
    t.handleTorpedoHit();
    t.handleTorpedoHit();
    t.handleTorpedoHit();
    TS_ASSERT_EQUALS(t.getMinFightersAboard(), 20);
    TS_ASSERT_EQUALS(t.getNumTorpedoHits(), 3);
    TS_ASSERT_EQUALS(t.getNumFights(), 1);

    // Merge
    game::vcr::Statistic other;
    other.init(obj, 1);
    other.handleTorpedoHit();
    other.handleFightersAboard(12);

    t.merge(other);
    TS_ASSERT_EQUALS(t.getMinFightersAboard(), 12);
    TS_ASSERT_EQUALS(t.getNumTorpedoHits(), 4);
    TS_ASSERT_EQUALS(t.getNumFights(), 2);
}

