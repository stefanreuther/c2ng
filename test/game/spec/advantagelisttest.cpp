/**
  *  \file test/game/spec/advantagelisttest.cpp
  *  \brief Test for game::spec::AdvantageList
  */

#include "game/spec/advantagelist.hpp"

#include "afl/test/testrunner.hpp"

using game::spec::AdvantageList;

AFL_TEST("game.spec.AdvantageList:empty", a)
{
    AdvantageList testee;

    a.checkNull("01. find",    testee.find(1));
    a.checkNull("02. getABI",  testee.getAdvantageByIndex(1));
    a.checkEqual("03. getNum", testee.getNumAdvantages(), 0U);

    // No-ops
    testee.setName(0, "foo");
    testee.setDescription(0, "bar");
    testee.addPlayer(0, 1);

    a.checkEqual("11. id",      testee.getId(0), 0);
    a.checkEqual("12. name",    testee.getName(0), "");
    a.checkEqual("13. desc",    testee.getDescription(0), "");
    a.checkEqual("14. players", testee.getPlayers(0), game::PlayerSet_t());
}


AFL_TEST("game.spec.AdvantageList:add", a)
{
    AdvantageList testee;

    // Add
    {
        AdvantageList::Item* p1 = testee.add(7);
        a.checkNonNull("01. add", p1);
        a.checkEqual("02. name", testee.getName(p1), "");
        a.checkEqual("03. desc", testee.getDescription(p1), "");
        testee.setName(p1, "seven");
        testee.setDescription(p1, "description for seven");
        testee.addPlayer(p1, 7);
    }

    {
        AdvantageList::Item* p2 = testee.add(23);
        a.checkNonNull("11. add", p2);
        testee.setName(p2, "twenty-three");
        testee.setDescription(p2, "more...");
        testee.addPlayer(p2, 2);
        testee.addPlayer(p2, 3);
    }

    // Find
    // (We do not guarantee pointers to be stable during adds)
    AdvantageList::Item* p1 = testee.find(7);
    AdvantageList::Item* p2 = testee.find(23);
    a.checkNonNull("21. find", p1);
    a.checkNonNull("22. find", p2);

    // Verify
    a.checkEqual("31. id",      testee.getId(p1), 7);
    a.checkEqual("32. name",    testee.getName(p1), "seven");
    a.checkEqual("33. desc",    testee.getDescription(p1), "description for seven");
    a.checkEqual("34. players", testee.getPlayers(p1), game::PlayerSet_t(7));

    a.checkEqual("41. id",      testee.getId(p2), 23);
    a.checkEqual("42. name",    testee.getName(p2), "twenty-three");
    a.checkEqual("43. desc",    testee.getDescription(p2), "more...");
    a.checkEqual("44. players", testee.getPlayers(p2), game::PlayerSet_t() + 2 + 3);

    a.checkEqual("51. num",     testee.getNumAdvantages(), 2U);
    a.checkEqual("52. get",     testee.getAdvantageByIndex(0), p1);
    a.checkEqual("53. get",     testee.getAdvantageByIndex(1), p2);

    // Failure
    a.checkNull("61. find",     testee.find(6));
    a.checkNull("62. get",      testee.getAdvantageByIndex(2));
}

AFL_TEST("game.spec.AdvantageList:add:repeated", a)
{
    AdvantageList testee;

    // Add
    AdvantageList::Item* p1 = testee.add(7);
    a.checkNonNull("01. add", p1);
    testee.setName(p1, "n");

    // Add again
    AdvantageList::Item* p2 = testee.add(7);
    a.checkNonNull("02. add", p2);
    a.checkEqual("03. name", testee.getName(p2), "n");
}
