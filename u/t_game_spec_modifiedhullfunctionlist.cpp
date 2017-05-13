/**
  *  \file u/t_game_spec_modifiedhullfunctionlist.cpp
  *  \brief Test for game::spec::ModifiedHullFunctionList
  */

#include "game/spec/modifiedhullfunctionlist.hpp"

#include "t_game_spec.hpp"

/** Simple tests. */
void
TestGameSpecModifiedHullFunctionList::testIt()
{
    using game::spec::ModifiedHullFunctionList;
    using game::spec::HullFunction;

    // Empty list
    ModifiedHullFunctionList testee;
    HullFunction fn;

    // Starts as 1:1 mapping
    TS_ASSERT_EQUALS(testee.getFunctionIdFromHostId(1), 1);
    TS_ASSERT_EQUALS(testee.getFunctionIdFromHostId(2), 2);

    TS_ASSERT(testee.getFunctionDefinition(1, fn));
    TS_ASSERT_EQUALS(fn.getBasicFunctionId(), 1);
    TS_ASSERT_EQUALS(fn.getKind(), fn.AssignedToShip);
    TS_ASSERT_EQUALS(fn.getPlayers(), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));

    // Add some things
    HullFunction fndef7(7, game::ExperienceLevelSet_t::allUpTo(3));
    ModifiedHullFunctionList::Function_t fnid7 = testee.getFunctionIdFromDefinition(fndef7);

    HullFunction fndef8(8, game::ExperienceLevelSet_t::allUpTo(4));
    fndef8.setHostId(42);
    ModifiedHullFunctionList::Function_t fnid8 = testee.getFunctionIdFromDefinition(fndef8);

    HullFunction fndef9(9, game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS));
    ModifiedHullFunctionList::Function_t fnid9 = testee.getFunctionIdFromDefinition(fndef9);

    // Verify function Ids
    TS_ASSERT_DIFFERS(fnid7, 7);
    TS_ASSERT_DIFFERS(fnid8, 8);
    TS_ASSERT_DIFFERS(fnid8, fnid7);
    TS_ASSERT_EQUALS(fnid9, 9);

    // Verify updated mapping
    TS_ASSERT_EQUALS(testee.getFunctionIdFromHostId(1), 1);
    TS_ASSERT_EQUALS(testee.getFunctionIdFromHostId(7), 7);
    TS_ASSERT_EQUALS(testee.getFunctionIdFromHostId(8), 8);
    TS_ASSERT_EQUALS(testee.getFunctionIdFromHostId(42), fnid8);

    TS_ASSERT(testee.getFunctionDefinition(fnid7, fn));
    TS_ASSERT_EQUALS(fn.getBasicFunctionId(), 7);
    TS_ASSERT_EQUALS(fn.getPlayers(), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
    TS_ASSERT_EQUALS(fn.getLevels(), game::ExperienceLevelSet_t::allUpTo(3));

    // Update with another definition of #7 to set the host Id
    HullFunction fndef7a(7, game::ExperienceLevelSet_t::allUpTo(3));
    fndef7a.setHostId(55);
    ModifiedHullFunctionList::Function_t fnid7a = testee.getFunctionIdFromDefinition(fndef7a);

    TS_ASSERT_EQUALS(fnid7a, fnid7);
    TS_ASSERT_EQUALS(testee.getFunctionIdFromHostId(55), fnid7);

    // Invalid request
    TS_ASSERT(!testee.getFunctionDefinition(-1, fn));

    // Clear invalidates
    testee.clear();
    TS_ASSERT_EQUALS(testee.getFunctionIdFromHostId(42), 42);
}

