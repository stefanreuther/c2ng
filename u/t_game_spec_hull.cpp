/**
  *  \file u/t_game_spec_hull.cpp
  *  \brief Test for game::spec::Hull
  */

#include "game/spec/hull.hpp"

#include "t_game_spec.hpp"

/** Accessor tests. */
void
TestGameSpecHull::testIt()
{
    game::spec::Hull h(7);

    // Initial state
    TS_ASSERT_EQUALS(h.getExternalPictureNumber(), 0);
    TS_ASSERT_EQUALS(h.getInternalPictureNumber(), 0);
    TS_ASSERT_EQUALS(h.getMaxFuel(), 0);
    TS_ASSERT_EQUALS(h.getMaxCrew(), 0);
    TS_ASSERT_EQUALS(h.getNumEngines(), 0);
    TS_ASSERT_EQUALS(h.getMaxCargo(), 0);
    TS_ASSERT_EQUALS(h.getNumBays(), 0);
    TS_ASSERT_EQUALS(h.getMaxLaunchers(), 0);
    TS_ASSERT_EQUALS(h.getMaxBeams(), 0);
    TS_ASSERT_EQUALS(h.getId(), 7);

    // Configure
    h.setExternalPictureNumber(230);
    h.setInternalPictureNumber(333);
    h.setMaxFuel(600);
    h.setMaxCrew(1200);
    h.setNumEngines(3);
    h.setMaxCargo(2400);
    h.setNumBays(4);
    h.setMaxLaunchers(2);
    h.setMaxBeams(12);

    // Verify
    TS_ASSERT_EQUALS(h.getExternalPictureNumber(), 230);
    TS_ASSERT_EQUALS(h.getInternalPictureNumber(), 333);
    TS_ASSERT_EQUALS(h.getMaxFuel(), 600);
    TS_ASSERT_EQUALS(h.getMaxCrew(), 1200);
    TS_ASSERT_EQUALS(h.getNumEngines(), 3);
    TS_ASSERT_EQUALS(h.getMaxCargo(), 2400);
    TS_ASSERT_EQUALS(h.getNumBays(), 4);
    TS_ASSERT_EQUALS(h.getMaxLaunchers(), 2);
    TS_ASSERT_EQUALS(h.getMaxBeams(), 12);
    TS_ASSERT_EQUALS(h.getId(), 7);
}

/** Test hull functions. */
void
TestGameSpecHull::testHullFunctions()
{
    game::spec::Hull h(88);
    const game::spec::Hull& ch(h);

    // General access
    TS_ASSERT_EQUALS(&h.getHullFunctions(true), &ch.getHullFunctions(true));
    TS_ASSERT_EQUALS(&h.getHullFunctions(false), &ch.getHullFunctions(false));
    TS_ASSERT_DIFFERS(&h.getHullFunctions(true), &h.getHullFunctions(false));

    // Functionality litmus test
    const game::spec::ModifiedHullFunctionList::Function_t fn(game::spec::ModifiedHullFunctionList::Function_t(333));
    
    h.changeHullFunction(fn, game::PlayerSet_t(1), game::PlayerSet_t(), true);
    TS_ASSERT(h.getHullFunctions(true).findEntry(fn) != 0);
    TS_ASSERT_EQUALS(ch.getHullFunctions(true).findEntry(fn), h.getHullFunctions(true).findEntry(fn));
    TS_ASSERT(h.getHullFunctions(false).findEntry(fn) == 0);

    h.clearHullFunctions();
    TS_ASSERT(h.getHullFunctions(true).findEntry(fn) == 0);
    TS_ASSERT(ch.getHullFunctions(true).findEntry(fn) == 0);
}
