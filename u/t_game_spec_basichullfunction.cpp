/**
  *  \file u/t_game_spec_basichullfunction.cpp
  *  \brief Test for game::spec::BasicHullFunction
  */

#include "game/spec/basichullfunction.hpp"

#include "t_game_spec.hpp"

/** Simple test of getters/setters. */
void
TestGameSpecBasicHullFunction::testIt()
{
    game::spec::BasicHullFunction testee(3, "Exterminate");

    // Initial state
    TS_ASSERT_EQUALS(testee.getId(), 3);
    TS_ASSERT_EQUALS(testee.getName(), "Exterminate");
    TS_ASSERT_EQUALS(testee.getDescription(), "Exterminate");
    TS_ASSERT_EQUALS(testee.getExplanation(), "");
    TS_ASSERT_EQUALS(testee.getImpliedFunctionId(), -1);

    // Change name; description follows as it's not set
    testee.setName("Extinguish");
    TS_ASSERT_EQUALS(testee.getName(), "Extinguish");
    TS_ASSERT_EQUALS(testee.getDescription(), "Extinguish");

    // Change more stuff
    testee.setDescription("Description");
    testee.setExplanation("Text");
    testee.setImpliedFunctionId(12);

    // Verify
    TS_ASSERT_EQUALS(testee.getName(), "Extinguish");
    TS_ASSERT_EQUALS(testee.getDescription(), "Description");
    TS_ASSERT_EQUALS(testee.getExplanation(), "Text");
    TS_ASSERT_EQUALS(testee.getImpliedFunctionId(), 12);
}

/** Test set/add explanation. */
void
TestGameSpecBasicHullFunction::testExplain()
{
    {
        game::spec::BasicHullFunction testee(4, "Fun");
        testee.addToExplanation("a");
        TS_ASSERT_EQUALS(testee.getExplanation(), "a");
    }
    {
        game::spec::BasicHullFunction testee(4, "Fun");
        testee.addToExplanation("a");
        testee.setExplanation("b");
        TS_ASSERT_EQUALS(testee.getExplanation(), "b");
    }
    {
        game::spec::BasicHullFunction testee(4, "Fun");
        testee.addToExplanation("a");
        testee.addToExplanation("b");
        TS_ASSERT_EQUALS(testee.getExplanation(), "a\nb");
    }
    {
        game::spec::BasicHullFunction testee(4, "Fun");
        testee.setExplanation("b");
        testee.addToExplanation("a");
        TS_ASSERT_EQUALS(testee.getExplanation(), "b\na");
    }
    {
        game::spec::BasicHullFunction testee(4, "Fun");
        testee.setExplanation("b\n");
        testee.addToExplanation("a");
        TS_ASSERT_EQUALS(testee.getExplanation(), "b\na");
    }
}
