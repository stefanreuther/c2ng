/**
  *  \file test/game/map/info/nulllinkbuildertest.cpp
  *  \brief Test for game::map::info::NullLinkBuilder
  */

#include "game/map/info/nulllinkbuilder.hpp"
#include "afl/test/testrunner.hpp"

/** Simple coverage test for completeness. */
AFL_TEST("game.map.info.NullLinkBuilder", a)
{
    game::map::info::NullLinkBuilder t;
    a.checkEqual("01. makePlanetLink", t.makePlanetLink(game::map::Planet(42)), "");
    a.checkEqual("02. makeSearchLink", t.makeSearchLink(game::SearchQuery()), "");
}
