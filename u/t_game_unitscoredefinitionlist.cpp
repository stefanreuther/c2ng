/**
  *  \file u/t_game_unitscoredefinitionlist.cpp
  *  \brief Test for game::UnitScoreDefinitionList
  */

#include "game/unitscoredefinitionlist.hpp"

#include "t_game.hpp"

void
TestGameUnitScoreDefinitionList::testIt()
{
    game::UnitScoreDefinitionList testee;
    TS_ASSERT_EQUALS(testee.getNumScores(), 0U);
    TS_ASSERT(testee.get(0) == 0);

    game::UnitScoreDefinitionList::Index_t found;
    TS_ASSERT(!testee.lookup(9, found));
    
    game::UnitScoreDefinitionList::Definition def = {
        "foo",
        9,
        1000
    };
    game::UnitScoreDefinitionList::Index_t ix = testee.add(def);
    TS_ASSERT_EQUALS(ix, testee.add(def));
    TS_ASSERT_EQUALS(ix, testee.add(def));
    TS_ASSERT_EQUALS(ix, testee.add(def));

    TS_ASSERT(testee.get(ix) != 0);
    TS_ASSERT_EQUALS(testee.get(ix)->name, "foo");
    TS_ASSERT_EQUALS(testee.get(ix)->id, 9);
    TS_ASSERT_EQUALS(testee.get(ix)->limit, 1000);

    TS_ASSERT(testee.lookup(9, found));
    TS_ASSERT_EQUALS(ix, found);
}
