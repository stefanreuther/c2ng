/**
  *  \file u/t_game_spec_friendlycode.cpp
  *  \brief Test for game::spec::FriendlyCode
  */

#include "game/spec/friendlycode.hpp"

#include "t_game_spec.hpp"
#include "game/playerlist.hpp"

void
TestGameSpecFriendlyCode::testFCode()
{
    // ex GameFcodeTestSuite::testFCode
    game::spec::FriendlyCode mkt("mkt", "sc,make torps");
    game::spec::FriendlyCode lfm("lfm", "sc+9ab,make fighters");
    game::spec::FriendlyCode att("ATT", "p,attack");

    game::PlayerList list;

    TS_ASSERT_EQUALS(mkt.getCode(), "mkt");
    TS_ASSERT(mkt.getRaces().contains(1));
    TS_ASSERT(mkt.getRaces().contains(2));
    TS_ASSERT(mkt.getRaces().contains(10));
    TS_ASSERT_EQUALS(mkt.getDescription(list), "make torps");

    TS_ASSERT(!lfm.getRaces().contains(1));
    TS_ASSERT(!lfm.getRaces().contains(8));
    TS_ASSERT(lfm.getRaces().contains(9));
    TS_ASSERT(lfm.getRaces().contains(10));
    TS_ASSERT(lfm.getRaces().contains(11));
}
