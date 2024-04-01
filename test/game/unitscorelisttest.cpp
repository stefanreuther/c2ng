/**
  *  \file test/game/unitscorelisttest.cpp
  *  \brief Test for game::UnitScoreList
  */

#include "game/unitscorelist.hpp"
#include "afl/test/testrunner.hpp"
#include "game/unitscoredefinitionlist.hpp"

/** Simple tests. */
AFL_TEST("game.UnitScoreList:basics", a)
{
    game::UnitScoreList testee;
    int16_t v, t;
    a.check("01. get", !testee.get(1, v, t));

    testee.set(1, 20, 10);
    a.check("11. get", testee.get(1, v, t));
    a.checkEqual("12. value", v, 20);
    a.checkEqual("13. turn", t, 10);

    a.check("21. get", !testee.get(0, v, t));
    a.check("22. get", !testee.get(2, v, t));

    testee.merge(1, 20, 5);
    a.check("31. get", testee.get(1, v, t));
    a.checkEqual("32. value", v, 20);
    a.checkEqual("33. turn", t, 10);

    testee.merge(3, 33, 3);
    a.check("41. get", testee.get(3, v, t));
    a.checkEqual("42. value", v, 33);
    a.checkEqual("43. turn", t, 3);
}

/** Test that a UnitScoreList is copyable. */
AFL_TEST("game.UnitScoreList:copy", a)
{
    int16_t v, t;

    // Make a list
    game::UnitScoreList testee;
    testee.set(1, 100, 9);

    // Copy it and verify that we can get the correct result
    game::UnitScoreList other(testee);
    a.check("01. get", other.get(1, v, t));
    a.checkEqual("02. value", v, 100);
    a.checkEqual("03. turn", t, 9);

    // Add a value
    other.set(4, 40, 4);
    a.check("11. get", other.get(4, v, t));

    // Assigning the original cancels the new value
    other = testee;
    a.check("21. get", !other.get(4, v, t));
}

/** Test merge(). */
AFL_TEST("game.UnitScoreList:merge", a)
{
    int16_t v, t;

    // Make a list
    game::UnitScoreList testee;
    testee.set(1, 100, 9);

    // Merge same turn
    testee.merge(1, 200, 9);
    a.check("01. get", testee.get(1, v, t));
    a.checkEqual("02. value", v, 200);
    a.checkEqual("03. turn", t, 9);

    // Merge older turn (ignored)
    testee.merge(1, 300, 4);
    a.check("11. get", testee.get(1, v, t));
    a.checkEqual("12. value", v, 200);
    a.checkEqual("13. turn", t, 9);

    // Merge newer turn
    testee.merge(1, 400, 11);
    a.check("21. get", testee.get(1, v, t));
    a.checkEqual("22. value", v, 400);
    a.checkEqual("23. turn", t, 11);
}

/** Test getScoreById(). */
AFL_TEST("game.UnitScoreList:getScoreById", a)
{
    game::UnitScoreDefinitionList defs;
    game::UnitScoreDefinitionList::Definition scoreDef;
    scoreDef.name  = "Level";
    scoreDef.id    = 77;
    scoreDef.limit = -1;

    game::UnitScoreList list;
    list.set(defs.add(scoreDef), 3, 44);

    a.checkEqual("01", list.getScoreById(77, defs).orElse(-1), 3);
    a.checkEqual("02", list.getScoreById(78, defs).isValid(), false);
}
