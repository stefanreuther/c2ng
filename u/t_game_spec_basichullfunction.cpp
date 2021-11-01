/**
  *  \file u/t_game_spec_basichullfunction.cpp
  *  \brief Test for game::spec::BasicHullFunction
  */

#include "game/spec/basichullfunction.hpp"

#include "t_game_spec.hpp"
#include "game/spec/hullfunction.hpp"

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
    TS_ASSERT_EQUALS(testee.getPictureName(), "");
    TS_ASSERT_EQUALS(testee.getImpliedFunctionId(), -1);

    // Change name; description follows as it's not set
    testee.setName("Extinguish");
    TS_ASSERT_EQUALS(testee.getName(), "Extinguish");
    TS_ASSERT_EQUALS(testee.getDescription(), "Extinguish");

    // Change more stuff
    testee.setDescription("Description");
    testee.setExplanation("Text");
    testee.setPictureName("boom");
    testee.setImpliedFunctionId(12);

    // Verify
    TS_ASSERT_EQUALS(testee.getName(), "Extinguish");
    TS_ASSERT_EQUALS(testee.getDescription(), "Description");
    TS_ASSERT_EQUALS(testee.getExplanation(), "Text");
    TS_ASSERT_EQUALS(testee.getPictureName(), "boom");
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

void
TestGameSpecBasicHullFunction::testGetDamageLimit()
{
    using game::spec::BasicHullFunction;
    using game::config::HostConfiguration;

    HostConfiguration defaultConfig;
    HostConfiguration otherConfig;
    otherConfig[HostConfiguration::DamageLevelForCloakFail].set(27);
    otherConfig[HostConfiguration::DamageLevelForAntiCloakFail].set(12);
    otherConfig[HostConfiguration::DamageLevelForChunnelFail].set(3);
    otherConfig[HostConfiguration::DamageLevelForTerraformFail].set(8);
    otherConfig[HostConfiguration::DamageLevelForHyperjumpFail].set(64);

    // Cloak (default config: 1)
    {
        BasicHullFunction testee(BasicHullFunction::Cloak, "Fun");
        TS_ASSERT_EQUALS(testee.getDamageLimit(1, defaultConfig).orElse(-1), 1);
        TS_ASSERT_EQUALS(testee.getDamageLimit(1, otherConfig).orElse(-1), 27);
    }

    // Anti-cloak (default config: 20)
    {
        BasicHullFunction testee(BasicHullFunction::LokiAnticloak, "Fun");
        TS_ASSERT_EQUALS(testee.getDamageLimit(1, defaultConfig).orElse(-1), 20);
        TS_ASSERT_EQUALS(testee.getDamageLimit(1, otherConfig).orElse(-1), 12);
    }

    // Hyperdrive (default config: 100)
    {
        BasicHullFunction testee(BasicHullFunction::Hyperdrive, "Fun");
        TS_ASSERT_EQUALS(testee.getDamageLimit(1, defaultConfig).orElse(-1), 100);
        TS_ASSERT_EQUALS(testee.getDamageLimit(1, otherConfig).orElse(-1), 64);
    }

    // Heat (default config: 100)
    {
        BasicHullFunction testee(BasicHullFunction::HeatsTo50, "Fun");
        TS_ASSERT_EQUALS(testee.getDamageLimit(1, defaultConfig).orElse(-1), 100);
        TS_ASSERT_EQUALS(testee.getDamageLimit(1, otherConfig).orElse(-1), 8);
    }

    // Chunnel (default config: 100)
    {
        BasicHullFunction testee(BasicHullFunction::ChunnelSelf, "Fun");
        TS_ASSERT_EQUALS(testee.getDamageLimit(1, defaultConfig).orElse(-1), 100);
        TS_ASSERT_EQUALS(testee.getDamageLimit(1, otherConfig).orElse(-1), 3);
    }

    // Imperial Assault (always 1)
    {
        BasicHullFunction testee(BasicHullFunction::ImperialAssault, "Fun");
        TS_ASSERT_EQUALS(testee.getDamageLimit(1, defaultConfig).orElse(-1), 1);
        TS_ASSERT_EQUALS(testee.getDamageLimit(1, otherConfig).orElse(-1), 1);
    }

    // Boarding (always unfailable)
    {
        BasicHullFunction testee(BasicHullFunction::Boarding, "Fun");
        TS_ASSERT(!testee.getDamageLimit(1, defaultConfig).isValid());
        TS_ASSERT(!testee.getDamageLimit(1, otherConfig).isValid());
    }
}

