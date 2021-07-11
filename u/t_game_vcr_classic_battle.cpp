/**
  *  \file u/t_game_vcr_classic_battle.cpp
  *  \brief Test for game::vcr::classic::Battle
  */

#include "game/vcr/classic/battle.hpp"

#include "t_game_vcr_classic.hpp"
#include "game/test/shiplist.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/string/nulltranslator.hpp"

namespace {
    game::vcr::Object makeLeftShip()
    {
        game::vcr::Object left;
        left.setMass(150);
        left.setCrew(2);
        left.setId(14);
        left.setOwner(2);
        left.setBeamType(0);
        left.setNumBeams(0);
        left.setNumBays(0);
        left.setTorpedoType(0);
        left.setNumLaunchers(0);
        left.setNumTorpedoes(0);
        left.setNumFighters(0);
        left.setShield(100);
        return left;
    }

    game::vcr::Object makeRightShip()
    {
        game::vcr::Object right;
        right.setMass(233);
        right.setCrew(240);
        right.setId(434);
        right.setOwner(3);
        right.setBeamType(5);
        right.setNumBeams(6);
        right.setNumBays(0);
        right.setTorpedoType(7);
        right.setNumLaunchers(4);
        right.setNumTorpedoes(0);
        right.setNumFighters(0);
        right.setShield(100);
        return right;
    }
}


void
TestGameVcrClassicBattle::testSample()
{
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);

    game::config::HostConfiguration config;

    afl::string::NullTranslator tx;

    // Configure from pcc-v2/tests/vcr/vcr2.dat #1
    game::vcr::classic::Battle t(makeLeftShip(), makeRightShip(), 42, 0, 0);
    t.setType(game::vcr::classic::Host, 0);

    // Verify
    game::map::Point pos;
    TS_ASSERT_EQUALS(t.getNumObjects(), 2U);
    TS_ASSERT_EQUALS(t.getObject(0, false)->getId(), 14);
    TS_ASSERT_EQUALS(t.getObject(1, false)->getId(), 434);
    TS_ASSERT_EQUALS(t.getObject(0, false)->getCrew(), 2);
    TS_ASSERT_EQUALS(t.getObject(1, false)->getCrew(), 240);
    TS_ASSERT(t.getObject(2, false) == 0);
    TS_ASSERT_EQUALS(t.getPosition(pos), false);
    TS_ASSERT_EQUALS(t.getAlgorithmName(tx), "Host");

    TS_ASSERT_EQUALS(t.getSignature(), 0);
    TS_ASSERT_EQUALS(t.getSeed(), 42);
    TS_ASSERT_EQUALS(t.getCapabilities(), 0);

    TS_ASSERT_EQUALS(t.getNumGroups(), 2U);

    TS_ASSERT_EQUALS(t.getGroupInfo(0, config).firstObject, 0U);
    TS_ASSERT_EQUALS(t.getGroupInfo(0, config).numObjects, 1U);
    TS_ASSERT_EQUALS(t.getGroupInfo(0, config).x, -29000);
    TS_ASSERT_EQUALS(t.getGroupInfo(0, config).y, 0);
    TS_ASSERT_EQUALS(t.getGroupInfo(0, config).owner, 2);
    TS_ASSERT_EQUALS(t.getGroupInfo(0, config).speed, 100);

    TS_ASSERT_EQUALS(t.getGroupInfo(1, config).firstObject, 1U);
    TS_ASSERT_EQUALS(t.getGroupInfo(1, config).numObjects, 1U);
    TS_ASSERT_EQUALS(t.getGroupInfo(1, config).x, 25000);
    TS_ASSERT_EQUALS(t.getGroupInfo(1, config).y, 0);
    TS_ASSERT_EQUALS(t.getGroupInfo(1, config).owner, 3);
    TS_ASSERT_EQUALS(t.getGroupInfo(1, config).speed, 100);

    // Prepare result
    t.prepareResult(config, shipList, game::vcr::Battle::NeedCompleteResult);
    TS_ASSERT_EQUALS(t.getObject(0, true)->getId(), 14);
    TS_ASSERT_EQUALS(t.getObject(1, true)->getId(), 434);
    TS_ASSERT_EQUALS(t.getObject(0, true)->getCrew(), 0);
    TS_ASSERT_EQUALS(t.getObject(1, true)->getCrew(), 240);
    TS_ASSERT_EQUALS(t.getOutcome(config, shipList, 0), 3);   // "captured by 3"
    TS_ASSERT_EQUALS(t.getOutcome(config, shipList, 1), 0);   // "survived"
    TS_ASSERT_EQUALS(t.getResultSummary(2, config, shipList, util::NumberFormatter(false, false), tx), "They have captured our ship.");
    TS_ASSERT_EQUALS(t.getResultSummary(3, config, shipList, util::NumberFormatter(false, false), tx), "We captured their ship.");
}

void
TestGameVcrClassicBattle::testPosition()
{
    // Configure from pcc-v2/tests/vcr/vcr2.dat #1
    game::vcr::classic::Battle t(makeLeftShip(), makeRightShip(), 42, 0, 0);
    t.setPosition(game::map::Point(500, 600));

    // Verify
    game::map::Point pos;
    TS_ASSERT_EQUALS(t.getPosition(pos), true);
    TS_ASSERT_EQUALS(pos.getX(), 500);
    TS_ASSERT_EQUALS(pos.getY(), 600);
}

void
TestGameVcrClassicBattle::testPoints()
{
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);

    game::config::HostConfiguration config;
    config[config.NumExperienceLevels].set(3);

    afl::string::NullTranslator tx;

    // Configure from pcc-v2/tests/vcr/vcr2.dat #1
    game::vcr::classic::Battle t(makeLeftShip(), makeRightShip(), 42, 0, 0);
    t.setType(game::vcr::classic::PHost4, 0);
    t.prepareResult(config, shipList, game::vcr::Battle::NeedCompleteResult);
    TS_ASSERT_EQUALS(t.getResultSummary(2, config, shipList, util::NumberFormatter(false, false), tx), "They have captured our ship (2 BP, 5 EP).");
    TS_ASSERT_EQUALS(t.getResultSummary(3, config, shipList, util::NumberFormatter(false, false), tx), "We captured their ship (2 BP, 5 EP).");

    // Points for each side
    {
        game::vcr::Score s;
        TS_ASSERT_EQUALS(t.computeScores(s, 0, config, shipList), true);
        TS_ASSERT_EQUALS(s.getBuildMillipoints().min(), 0);
        TS_ASSERT_EQUALS(s.getBuildMillipoints().max(), 0);
        TS_ASSERT_EQUALS(s.getExperience().min(), 0);
        TS_ASSERT_EQUALS(s.getExperience().max(), 0);
        TS_ASSERT_EQUALS(s.getTonsDestroyed().min(), 0);
        TS_ASSERT_EQUALS(s.getTonsDestroyed().max(), 0);
    }
    {
        game::vcr::Score s;
        TS_ASSERT_EQUALS(t.computeScores(s, 1, config, shipList), true);
        TS_ASSERT_EQUALS(s.getBuildMillipoints().min(), 2200);
        TS_ASSERT_EQUALS(s.getBuildMillipoints().max(), 2200);
        TS_ASSERT_EQUALS(s.getExperience().min(), 5);
        TS_ASSERT_EQUALS(s.getExperience().max(), 5);
        TS_ASSERT_EQUALS(s.getTonsDestroyed().min(), 0);
        TS_ASSERT_EQUALS(s.getTonsDestroyed().max(), 0);
    }
}

void
TestGameVcrClassicBattle::testPointsRange()
{
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);

    game::config::HostConfiguration config;
    config[config.NumExperienceLevels].set(3);
    config[config.PALCombatAggressor].set(12);
    config[config.PALOpponentPointsPer10KT].set(5);
    config[config.PALAggressorPointsPer10KT].set(10);

    afl::string::NullTranslator tx;
    util::NumberFormatter fmt(false, false);

    // Standard / role not known
    {
        game::vcr::classic::Battle t(makeLeftShip(), makeRightShip(), 42, 0, 0);
        t.setType(game::vcr::classic::PHost4, 0);
        t.prepareResult(config, shipList, game::vcr::Battle::NeedCompleteResult);
        TS_ASSERT_EQUALS(t.getResultSummary(3, config, shipList, fmt, tx), "We captured their ship (4 ... 19 BP, 5 EP).");
    }

    // We know that captor is aggressor
    {
        game::vcr::Object obj(makeRightShip());
        obj.setRole(game::vcr::Object::AggressorRole);
        game::vcr::classic::Battle t(makeLeftShip(), obj, 42, 0, 0);
        t.setType(game::vcr::classic::PHost4, 0);
        t.prepareResult(config, shipList, game::vcr::Battle::NeedCompleteResult);
        TS_ASSERT_EQUALS(t.getResultSummary(3, config, shipList, fmt, tx), "We captured their ship (19 BP, 5 EP).");
    }

    // We know that captor is opponent
    {
        game::vcr::Object obj(makeRightShip());
        obj.setRole(game::vcr::Object::OpponentRole);
        game::vcr::classic::Battle t(makeLeftShip(), obj, 42, 0, 0);
        t.setType(game::vcr::classic::PHost4, 0);
        t.prepareResult(config, shipList, game::vcr::Battle::NeedCompleteResult);
        TS_ASSERT_EQUALS(t.getResultSummary(3, config, shipList, fmt, tx), "We captured their ship (4 BP, 5 EP).");
    }
}

