/**
  *  \file test/game/unitscoredefinitionlisttest.cpp
  *  \brief Test for game::UnitScoreDefinitionList
  */

#include "game/unitscoredefinitionlist.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("game.UnitScoreDefinitionList:basics", a)
{
    game::UnitScoreDefinitionList testee;
    a.checkEqual("01. getNumScores", testee.getNumScores(), 0U);
    a.checkNull("02. get", testee.get(0));

    game::UnitScoreDefinitionList::Index_t found;
    a.check("11. lookup", !testee.lookup(9, found));

    game::UnitScoreDefinitionList::Definition def = {
        "foo",
        9,
        1000
    };
    game::UnitScoreDefinitionList::Index_t ix = testee.add(def);
    a.checkEqual("21. add", ix, testee.add(def));
    a.checkEqual("22. add", ix, testee.add(def));
    a.checkEqual("23. add", ix, testee.add(def));

    a.checkNonNull("31. get", testee.get(ix));
    a.checkEqual("32. name",  testee.get(ix)->name, "foo");
    a.checkEqual("33. id",    testee.get(ix)->id, 9);
    a.checkEqual("34. limit", testee.get(ix)->limit, 1000);

    a.check("41. lookup", testee.lookup(9, found));
    a.checkEqual("42. found", ix, found);
}

/** UnitScoreDefinitionList must be copyable. */
AFL_TEST("game.UnitScoreDefinitionList:copy", a)
{
    // Set up
    game::UnitScoreDefinitionList testee;
    game::UnitScoreDefinitionList::Definition def = {
        "foo",
        9,
        1000
    };
    game::UnitScoreDefinitionList::Index_t ix = testee.add(def);

    // Copy
    game::UnitScoreDefinitionList other(testee);
    a.checkNonNull("01. get", other.get(ix));
    a.checkEqual("02. name", other.get(ix)->name, "foo");

    // Assign
    other = game::UnitScoreDefinitionList();
    a.checkNull("11. get", other.get(ix));
}
