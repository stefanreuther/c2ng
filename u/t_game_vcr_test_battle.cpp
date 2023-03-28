/**
  *  \file u/t_game_vcr_test_battle.cpp
  *  \brief Test for game::vcr::test::Battle
  */

#include "game/vcr/test/battle.hpp"

#include "t_game_vcr_test.hpp"
#include "game/spec/shiplist.hpp"
#include "game/config/hostconfiguration.hpp"
#include "afl/string/nulltranslator.hpp"
#include "util/numberformatter.hpp"
#include "game/vcr/score.hpp"
#include "game/vcr/object.hpp"

/** General tests. */
void
TestGameVcrTestBattle::testIt()
{
    // Environment
    game::config::HostConfiguration config;
    game::spec::ShipList shipList;
    afl::string::NullTranslator tx;
    util::NumberFormatter fmt(false, false);
    game::vcr::Score score;

    // Testee
    game::vcr::test::Battle testee;

    // Verify initial status
    TS_ASSERT_EQUALS(testee.getNumObjects(), 0U);
    TS_ASSERT_EQUALS(testee.getNumGroups(), 0U);
    TS_ASSERT_EQUALS(testee.getPlayability(config, shipList), game::vcr::Battle::IsPlayable);
    TS_ASSERT_EQUALS(testee.getAlgorithmName(tx), "Test");
    TS_ASSERT_EQUALS(testee.isESBActive(config), false);
    TS_ASSERT(!testee.getPosition().isValid());
    TS_ASSERT(!testee.getAuxiliaryInformation(game::vcr::Battle::aiSeed).isValid());
    TS_ASSERT_EQUALS(testee.getResultSummary(1, config, shipList, fmt, tx), "");
    TS_ASSERT(testee.getObject(0, false) == 0);

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
    TS_ASSERT_THROWS_NOTHING(testee.prepareResult(config, shipList, 0));
    TS_ASSERT_EQUALS(testee.computeScores(score, 0, config, shipList), false);

    // Verify
    TS_ASSERT_EQUALS(testee.getNumObjects(), 2U);

    TS_ASSERT(testee.getObject(1, false) != 0);
    TS_ASSERT_EQUALS(testee.getObject(1, false), static_cast<const game::vcr::Battle&>(testee).getObject(1, false));
    TS_ASSERT_DIFFERS(testee.getObject(1, false), testee.getObject(1, true));
    TS_ASSERT_EQUALS(testee.getObject(1, false)->getId(), 7);

    TS_ASSERT_EQUALS(testee.getNumGroups(), 2U);
    TS_ASSERT_EQUALS(testee.getGroupInfo(0, config).owner, 3);
    TS_ASSERT_EQUALS(testee.getOutcome(config, shipList, 0), 5);
    TS_ASSERT_EQUALS(testee.getPlayability(config, shipList), game::vcr::Battle::IsDamaged);
    TS_ASSERT_EQUALS(testee.getAlgorithmName(tx), "testIt");
    TS_ASSERT_EQUALS(testee.isESBActive(config), true);
    TS_ASSERT_EQUALS(testee.getPosition().orElse(game::map::Point()), game::map::Point(1300, 1200));
    TS_ASSERT_EQUALS(testee.getAuxiliaryInformation(game::vcr::Battle::aiSeed).orElse(-1), 1337);
    TS_ASSERT_EQUALS(testee.getAuxiliaryInformation(game::vcr::Battle::aiAmbient).isValid(), false);

    // Out-of-range access
    TS_ASSERT_EQUALS(testee.getOutcome(config, shipList, 7), 0);
    TS_ASSERT_EQUALS(testee.getGroupInfo(7, config).owner, 0);
}

/** Test manually configured groups. */
void
TestGameVcrTestBattle::testGroups()
{
    // Environment
    game::config::HostConfiguration config;

    // Test battle with some objects
    game::vcr::test::Battle testee;
    for (int i = 0; i < 10; ++i) {
        testee.addObject(game::vcr::Object(), 0);
    }

    // Explicitly add groups
    testee.addGroup(game::vcr::GroupInfo(0, 3, 1000, 100, 5, 20));
    testee.addGroup(game::vcr::GroupInfo(3, 7, 2000, 400, 9, 15));

    // Verify
    TS_ASSERT_EQUALS(testee.getNumObjects(), 10U);
    TS_ASSERT_EQUALS(testee.getNumGroups(), 2U);
    TS_ASSERT_EQUALS(testee.getGroupInfo(0, config).owner, 5);
    TS_ASSERT_EQUALS(testee.getGroupInfo(0, config).speed, 20);
    TS_ASSERT_EQUALS(testee.getGroupInfo(1, config).owner, 9);
    TS_ASSERT_EQUALS(testee.getGroupInfo(1, config).speed, 15);

    // Out-of-range access
    TS_ASSERT_EQUALS(testee.getGroupInfo(7, config).owner, 0);
}

