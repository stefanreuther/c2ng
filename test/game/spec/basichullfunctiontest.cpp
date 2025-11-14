/**
  *  \file test/game/spec/basichullfunctiontest.cpp
  *  \brief Test for game::spec::BasicHullFunction
  */

#include "game/spec/basichullfunction.hpp"

#include "afl/test/testrunner.hpp"
#include "game/spec/hullfunction.hpp"

/** Simple test of getters/setters. */
AFL_TEST("game.spec.BasicHullFunction:basics", a)
{
    game::spec::BasicHullFunction testee(3, "Exterminate");

    // Initial state
    a.checkEqual("01. getId",                testee.getId(), 3);
    a.checkEqual("02. getName",              testee.getName(), "Exterminate");
    a.checkEqual("03. getCode",              testee.getCode(), "");
    a.checkEqual("04. getDescription",       testee.getDescription(), "Exterminate");
    a.checkEqual("05. getExplanation",       testee.getExplanation(), "");
    a.checkEqual("06. getPictureName",       testee.getPictureName(), "");
    a.checkEqual("07. getImpliedFunctionId", testee.getImpliedFunctionId(), -1);

    // Change name; description follows as it's not set
    testee.setName("Extinguish");
    a.checkEqual("11. getName",        testee.getName(), "Extinguish");
    a.checkEqual("12. getDescription", testee.getDescription(), "Extinguish");

    // Change more stuff
    testee.setDescription("Description");
    testee.setExplanation("Text");
    testee.setPictureName("boom");
    testee.setCode("Ex");
    testee.setImpliedFunctionId(12);

    // Verify
    a.checkEqual("21. getName",              testee.getName(), "Extinguish");
    a.checkEqual("22. getCode",              testee.getCode(), "Ex");
    a.checkEqual("23. getDescription",       testee.getDescription(), "Description");
    a.checkEqual("24. getExplanation",       testee.getExplanation(), "Text");
    a.checkEqual("25. getPictureName",       testee.getPictureName(), "boom");
    a.checkEqual("26. getImpliedFunctionId", testee.getImpliedFunctionId(), 12);
}

/** Test set/add explanation. */

AFL_TEST("game.spec.BasicHullFunction:explanation:add", a)
{
    game::spec::BasicHullFunction testee(4, "Fun");
    testee.addToExplanation("a");
    a.checkEqual("getExplanation", testee.getExplanation(), "a");
}

AFL_TEST("game.spec.BasicHullFunction:explanation:add+set", a)
{
    game::spec::BasicHullFunction testee(4, "Fun");
    testee.addToExplanation("a");
    testee.setExplanation("b");
    a.checkEqual("getExplanation", testee.getExplanation(), "b");
}

AFL_TEST("game.spec.BasicHullFunction:explanation:add+add", a)
{
    game::spec::BasicHullFunction testee(4, "Fun");
    testee.addToExplanation("a");
    testee.addToExplanation("b");
    a.checkEqual("getExplanation", testee.getExplanation(), "a\nb");
}

AFL_TEST("game.spec.BasicHullFunction:explanation:set+add", a)
{
    game::spec::BasicHullFunction testee(4, "Fun");
    testee.setExplanation("b");
    testee.addToExplanation("a");
    a.checkEqual("getExplanation", testee.getExplanation(), "b\na");
}

AFL_TEST("game.spec.BasicHullFunction:explanation:set-with-newline+add", a)
{
    game::spec::BasicHullFunction testee(4, "Fun");
    testee.setExplanation("b\n");
    testee.addToExplanation("a");
    a.checkEqual("getExplanation", testee.getExplanation(), "b\na");
}

AFL_TEST("game.spec.BasicHullFunction:getDamageLimit", a)
{
    using afl::base::Ref;
    using game::spec::BasicHullFunction;
    using game::config::HostConfiguration;

    Ref<HostConfiguration> rdefault = HostConfiguration::create();
    Ref<HostConfiguration> rother = HostConfiguration::create();
    HostConfiguration& defaultConfig = *rdefault;
    HostConfiguration& otherConfig = *rother;
    otherConfig[HostConfiguration::DamageLevelForCloakFail].set(27);
    otherConfig[HostConfiguration::DamageLevelForAntiCloakFail].set(12);
    otherConfig[HostConfiguration::DamageLevelForChunnelFail].set(3);
    otherConfig[HostConfiguration::DamageLevelForTerraformFail].set(8);
    otherConfig[HostConfiguration::DamageLevelForHyperjumpFail].set(64);

    // Cloak (default config: 1)
    {
        BasicHullFunction testee(BasicHullFunction::Cloak, "Fun");
        a.checkEqual("01", testee.getDamageLimit(1, defaultConfig).orElse(-1), 1);
        a.checkEqual("02", testee.getDamageLimit(1, otherConfig).orElse(-1), 27);
    }

    // Anti-cloak (default config: 20)
    {
        BasicHullFunction testee(BasicHullFunction::LokiAnticloak, "Fun");
        a.checkEqual("11", testee.getDamageLimit(1, defaultConfig).orElse(-1), 20);
        a.checkEqual("12", testee.getDamageLimit(1, otherConfig).orElse(-1), 12);
    }

    // Hyperdrive (default config: 100)
    {
        BasicHullFunction testee(BasicHullFunction::Hyperdrive, "Fun");
        a.checkEqual("21", testee.getDamageLimit(1, defaultConfig).orElse(-1), 100);
        a.checkEqual("22", testee.getDamageLimit(1, otherConfig).orElse(-1), 64);
    }

    // Heat (default config: 100)
    {
        BasicHullFunction testee(BasicHullFunction::HeatsTo50, "Fun");
        a.checkEqual("31", testee.getDamageLimit(1, defaultConfig).orElse(-1), 100);
        a.checkEqual("32", testee.getDamageLimit(1, otherConfig).orElse(-1), 8);
    }

    // Chunnel (default config: 100)
    {
        BasicHullFunction testee(BasicHullFunction::ChunnelSelf, "Fun");
        a.checkEqual("41", testee.getDamageLimit(1, defaultConfig).orElse(-1), 100);
        a.checkEqual("42", testee.getDamageLimit(1, otherConfig).orElse(-1), 3);
    }

    // Imperial Assault (always 1)
    {
        BasicHullFunction testee(BasicHullFunction::ImperialAssault, "Fun");
        a.checkEqual("51", testee.getDamageLimit(1, defaultConfig).orElse(-1), 1);
        a.checkEqual("52", testee.getDamageLimit(1, otherConfig).orElse(-1), 1);
    }

    // Boarding (always unfailable)
    {
        BasicHullFunction testee(BasicHullFunction::Boarding, "Fun");
        a.check("61", !testee.getDamageLimit(1, defaultConfig).isValid());
        a.check("62", !testee.getDamageLimit(1, otherConfig).isValid());
    }
}
