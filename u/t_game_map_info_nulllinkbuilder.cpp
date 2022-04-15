/**
  *  \file u/t_game_map_info_nulllinkbuilder.cpp
  *  \brief Test for game::map::info::NullLinkBuilder
  */

#include "game/map/info/nulllinkbuilder.hpp"

#include "t_game_map_info.hpp"

/** Simple coverage test for completeness. */
void
TestGameMapInfoNullLinkBuilder::testIt()
{
    game::map::info::NullLinkBuilder t;
    TS_ASSERT_EQUALS(t.makePlanetLink(game::map::Planet(42)), "");
    TS_ASSERT_EQUALS(t.makeSearchLink(game::SearchQuery()), "");
}

