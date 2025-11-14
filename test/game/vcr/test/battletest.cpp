/**
  *  \file test/game/vcr/test/battletest.cpp
  *  \brief Test for game::vcr::test::Battle
  */

#include "game/vcr/test/battle.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"
#include "game/vcr/object.hpp"
#include "game/vcr/score.hpp"
#include "util/numberformatter.hpp"

using afl::base::Ref;
using game::config::HostConfiguration;

/** General tests. */
AFL_TEST("game.vcr.test.Battle:basics", a)
{
    // Environment
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    game::spec::ShipList shipList;
    afl::string::NullTranslator tx;
    util::NumberFormatter fmt(false, false);
    game::vcr::Score score;

    // Testee
    game::vcr::test::Battle testee;

    // Verify initial status
    a.checkEqual("01. getNumObjects",            testee.getNumObjects(), 0U);
    a.checkEqual("02. getNumGroups",             testee.getNumGroups(), 0U);
    a.checkEqual("03. getPlayability",           testee.getPlayability(config, shipList), game::vcr::Battle::IsPlayable);
    a.checkEqual("04. getAlgorithmName",         testee.getAlgorithmName(tx), "Test");
    a.checkEqual("05. isESBActive",              testee.isESBActive(config), false);
    a.check     ("06. getPosition",             !testee.getPosition().isValid());
    a.check     ("07. getAuxiliaryInformation", !testee.getAuxiliaryInformation(game::vcr::Battle::aiSeed).isValid());
    a.checkEqual("08. getResultSummary",         testee.getResultSummary(1, config, shipList, fmt, tx), "");
    a.checkNull ("09. getObject",                testee.getObject(0, false));

    // Add units
    game::vcr::Object o1;
    o1.setId(1);
    o1.setOwner(3);
    testee.addObject(o1, 5);

    game::vcr::Object o7;
    o7.setId(7);
    o7.setOwner(5);
    testee.addObject(o7, 0);

    // Configure
    testee.setPlayability(game::vcr::Battle::IsDamaged);
    testee.setAlgorithmName("testIt");
    testee.setIsESBActive(true);
    testee.setPosition(game::map::Point(1300, 1200));
    testee.setAuxiliaryInformation(game::vcr::Battle::aiSeed, 1337);
    AFL_CHECK_SUCCEEDS(a("11. prepareResult"), testee.prepareResult(config, shipList, 0));
    a.checkEqual("12. computeScores", testee.computeScores(score, 0, config, shipList), false);

    // Verify
    a.checkEqual("21. getNumObjects", testee.getNumObjects(), 2U);

    a.checkNonNull  ("31. getObject", testee.getObject(1, false));
    a.checkEqual    ("32. getObject", testee.getObject(1, false), static_cast<const game::vcr::Battle&>(testee).getObject(1, false));
    a.checkDifferent("33. getObject", testee.getObject(1, false), testee.getObject(1, true));
    a.checkEqual    ("34. getObject", testee.getObject(1, false)->getId(), 7);

    a.checkEqual("41. getNumGroups",            testee.getNumGroups(), 2U);
    a.checkEqual("42. getGroupInfo",            testee.getGroupInfo(0, config).owner, 3);
    a.checkEqual("43. getOutcome",              testee.getOutcome(config, shipList, 0), 5);
    a.checkEqual("44. getPlayability",          testee.getPlayability(config, shipList), game::vcr::Battle::IsDamaged);
    a.checkEqual("45. getAlgorithmName",        testee.getAlgorithmName(tx), "testIt");
    a.checkEqual("46. isESBActive",             testee.isESBActive(config), true);
    a.checkEqual("47. getPosition",             testee.getPosition().orElse(game::map::Point()), game::map::Point(1300, 1200));
    a.checkEqual("48. getAuxiliaryInformation", testee.getAuxiliaryInformation(game::vcr::Battle::aiSeed).orElse(-1), 1337);
    a.checkEqual("49. getAuxiliaryInformation", testee.getAuxiliaryInformation(game::vcr::Battle::aiAmbient).isValid(), false);

    // Out-of-range access
    a.checkEqual("51. getOutcome", testee.getOutcome(config, shipList, 7), 0);
    a.checkEqual("52. getGroupInfo", testee.getGroupInfo(7, config).owner, 0);
}

/** Test manually configured groups. */
AFL_TEST("game.vcr.test.Battle:groups", a)
{
    // Environment
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;

    // Test battle with some objects
    game::vcr::test::Battle testee;
    for (int i = 0; i < 10; ++i) {
        testee.addObject(game::vcr::Object(), 0);
    }

    // Explicitly add groups
    testee.addGroup(game::vcr::GroupInfo(0, 3, 1000, 100, 5, 20));
    testee.addGroup(game::vcr::GroupInfo(3, 7, 2000, 400, 9, 15));

    // Verify
    a.checkEqual("01. getNumObjects", testee.getNumObjects(), 10U);
    a.checkEqual("02. getNumGroups", testee.getNumGroups(), 2U);
    a.checkEqual("03. getGroupInfo", testee.getGroupInfo(0, config).owner, 5);
    a.checkEqual("04. getGroupInfo", testee.getGroupInfo(0, config).speed, 20);
    a.checkEqual("05. getGroupInfo", testee.getGroupInfo(1, config).owner, 9);
    a.checkEqual("06. getGroupInfo", testee.getGroupInfo(1, config).speed, 15);

    // Out-of-range access
    a.checkEqual("11. getGroupInfo", testee.getGroupInfo(7, config).owner, 0);
}
