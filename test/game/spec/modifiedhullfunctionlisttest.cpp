/**
  *  \file test/game/spec/modifiedhullfunctionlisttest.cpp
  *  \brief Test for game::spec::ModifiedHullFunctionList
  */

#include "game/spec/modifiedhullfunctionlist.hpp"
#include "afl/test/testrunner.hpp"

/** Simple tests. */
AFL_TEST("game.spec.ModifiedHullFunctionList", a)
{
    using game::spec::ModifiedHullFunctionList;
    using game::spec::HullFunction;

    // Empty list
    ModifiedHullFunctionList testee;
    HullFunction fn;

    // Starts as 1:1 mapping
    a.checkEqual("01. getFunctionIdFromHostId", testee.getFunctionIdFromHostId(1), 1);
    a.checkEqual("02. getFunctionIdFromHostId", testee.getFunctionIdFromHostId(2), 2);

    a.check("11. getFunctionDefinition", testee.getFunctionDefinition(1, fn));
    a.checkEqual("12. getBasicFunctionId", fn.getBasicFunctionId(), 1);
    a.checkEqual("13. getKind", fn.getKind(), fn.AssignedToShip);
    a.checkEqual("14. getPlayers", fn.getPlayers(), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));

    // Add some things
    HullFunction fndef7(7, game::ExperienceLevelSet_t::allUpTo(3));
    ModifiedHullFunctionList::Function_t fnid7 = testee.getFunctionIdFromDefinition(fndef7);

    HullFunction fndef8(8, game::ExperienceLevelSet_t::allUpTo(4));
    fndef8.setHostId(42);
    ModifiedHullFunctionList::Function_t fnid8 = testee.getFunctionIdFromDefinition(fndef8);

    HullFunction fndef9(9, game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS));
    ModifiedHullFunctionList::Function_t fnid9 = testee.getFunctionIdFromDefinition(fndef9);

    // Verify function Ids
    a.checkDifferent("21. getFunctionIdFromDefinition", fnid7, 7);
    a.checkDifferent("22. getFunctionIdFromDefinition", fnid8, 8);
    a.checkDifferent("23. getFunctionIdFromDefinition", fnid8, fnid7);
    a.checkEqual("24. Id matches host Id", fnid9, 9);

    // Verify updated mapping
    a.checkEqual("31. getFunctionIdFromHostId", testee.getFunctionIdFromHostId(1), 1);
    a.checkEqual("32. getFunctionIdFromHostId", testee.getFunctionIdFromHostId(7), 7);
    a.checkEqual("33. getFunctionIdFromHostId", testee.getFunctionIdFromHostId(8), 8);
    a.checkEqual("34. getFunctionIdFromHostId", testee.getFunctionIdFromHostId(42), fnid8);

    a.check("41. getFunctionDefinition", testee.getFunctionDefinition(fnid7, fn));
    a.checkEqual("42. getBasicFunctionId", fn.getBasicFunctionId(), 7);
    a.checkEqual("43. getPlayers", fn.getPlayers(), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
    a.checkEqual("44. getLevels", fn.getLevels(), game::ExperienceLevelSet_t::allUpTo(3));

    // Update with another definition of #7 to set the host Id
    HullFunction fndef7a(7, game::ExperienceLevelSet_t::allUpTo(3));
    fndef7a.setHostId(55);
    ModifiedHullFunctionList::Function_t fnid7a = testee.getFunctionIdFromDefinition(fndef7a);

    a.checkEqual("51. getFunctionIdFromDefinition", fnid7a, fnid7);
    a.checkEqual("52. getFunctionIdFromHostId", testee.getFunctionIdFromHostId(55), fnid7);

    // Invalid request
    a.check("61. getFunctionDefinition", !testee.getFunctionDefinition(-1, fn));

    // Clear invalidates
    testee.clear();
    a.checkEqual("71. getFunctionIdFromHostId", testee.getFunctionIdFromHostId(42), 42);
}
