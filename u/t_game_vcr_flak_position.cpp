/**
  *  \file u/t_game_vcr_flak_position.cpp
  *  \brief Test for game::vcr::flak::Position
  */

#include "game/vcr/flak/position.hpp"

#include "t_game_vcr_flak.hpp"

void
TestGameVcrFlakPosition::testIt()
{
    game::vcr::flak::Position a;
    game::vcr::flak::Position b(1000, 2000, 3000);
    game::vcr::flak::Position c(1500, 2000, 9000);
    game::vcr::flak::Position d(4000, 5000, 3000);

    TS_ASSERT_EQUALS(a.x, 0);
    TS_ASSERT_EQUALS(b.x, 1000);

    TS_ASSERT_EQUALS(a.distanceTo(a), 0.0);
    TS_ASSERT_EQUALS(b.distanceTo(b), 0.0);
    TS_ASSERT_EQUALS(b.distanceTo(c), 500.0);

    TS_ASSERT_EQUALS(a.isDistanceLERadius(a, 10), true);
    TS_ASSERT_EQUALS(b.isDistanceLERadius(c, 500), true);
    TS_ASSERT_EQUALS(b.isDistanceLERadius(c, 499), false);
    TS_ASSERT_EQUALS(b.isDistanceLERadius(d, 4242), false);
    TS_ASSERT_EQUALS(b.isDistanceLERadius(d, 4243), true);

    TS_ASSERT_EQUALS(a == a, true);
    TS_ASSERT_EQUALS(a != a, false);
    TS_ASSERT_EQUALS(a == b, false);
    TS_ASSERT_EQUALS(a != b, true);
}

