/**
  *  \file test/game/vcr/classic/battletest.cpp
  *  \brief Test for game::vcr::classic::Battle
  */

#include "game/vcr/classic/battle.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/shiplist.hpp"

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


AFL_TEST("game.vcr.classic.Battle:sample", a)
{
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);

    game::config::HostConfiguration config;

    afl::string::NullTranslator tx;

    // Configure from pcc-v2/tests/vcr/vcr2.dat #1
    game::vcr::classic::Battle t(makeLeftShip(), makeRightShip(), 42, 0);
    t.setType(game::vcr::classic::Host, 0);

    // Verify
    game::map::Point pos;
    a.checkEqual("01. getNumObjects",    t.getNumObjects(), 2U);
    a.checkEqual("02. getId",            t.getObject(0, false)->getId(), 14);
    a.checkEqual("03. getId",            t.getObject(1, false)->getId(), 434);
    a.checkEqual("04. getCrew",          t.getObject(0, false)->getCrew(), 2);
    a.checkEqual("05. getCrew",          t.getObject(1, false)->getCrew(), 240);
    a.checkNull ("06. getObject",        t.getObject(2, false));
    a.checkEqual("07. getPosition",      t.getPosition().get(pos), false);
    a.checkEqual("08. getAlgorithmName", t.getAlgorithmName(tx), "Host");

    a.checkEqual("11. getSignature",     t.getSignature(), 0);
    a.checkEqual("12. getSeed",          t.getSeed(), 42);
    a.checkEqual("13. getCapabilities",  t.getCapabilities(), 0);

    a.checkEqual("21. getNumGroups",     t.getNumGroups(), 2U);

    a.checkEqual("31. firstObject",      t.getGroupInfo(0, config).firstObject, 0U);
    a.checkEqual("32. numObjects",       t.getGroupInfo(0, config).numObjects, 1U);
    a.checkEqual("33. x",                t.getGroupInfo(0, config).x, -29000);
    a.checkEqual("34. y",                t.getGroupInfo(0, config).y, 0);
    a.checkEqual("35. owner",            t.getGroupInfo(0, config).owner, 2);
    a.checkEqual("36. speed",            t.getGroupInfo(0, config).speed, 100);

    a.checkEqual("41. firstObject",      t.getGroupInfo(1, config).firstObject, 1U);
    a.checkEqual("42. numObjects",       t.getGroupInfo(1, config).numObjects, 1U);
    a.checkEqual("43. x",                t.getGroupInfo(1, config).x, 25000);
    a.checkEqual("44. y",                t.getGroupInfo(1, config).y, 0);
    a.checkEqual("45. owner",            t.getGroupInfo(1, config).owner, 3);
    a.checkEqual("46. speed",            t.getGroupInfo(1, config).speed, 100);

    // Prepare result
    t.prepareResult(config, shipList, game::vcr::Battle::NeedCompleteResult);
    a.checkEqual("51. getId",            t.getObject(0, true)->getId(), 14);
    a.checkEqual("52. getId",            t.getObject(1, true)->getId(), 434);
    a.checkEqual("53. getCrew",          t.getObject(0, true)->getCrew(), 0);
    a.checkEqual("54. getCrew",          t.getObject(1, true)->getCrew(), 240);
    a.checkEqual("55. getOutcome",       t.getOutcome(config, shipList, 0), 3);   // "captured by 3"
    a.checkEqual("56. getOutcome",       t.getOutcome(config, shipList, 1), 0);   // "survived"
    a.checkEqual("57. getResultSummary", t.getResultSummary(2, config, shipList, util::NumberFormatter(false, false), tx), "They have captured our ship.");
    a.checkEqual("58. getResultSummary", t.getResultSummary(3, config, shipList, util::NumberFormatter(false, false), tx), "We captured their ship.");
}

AFL_TEST("game.vcr.classic.Battle:getPosition", a)
{
    // Configure from pcc-v2/tests/vcr/vcr2.dat #1
    game::vcr::classic::Battle t(makeLeftShip(), makeRightShip(), 42, 0);
    t.setPosition(game::map::Point(500, 600));

    // Verify
    game::map::Point pos;
    a.checkEqual("01. getPosition", t.getPosition().get(pos), true);
    a.checkEqual("02. getX", pos.getX(), 500);
    a.checkEqual("03. getY", pos.getY(), 600);
}

AFL_TEST("game.vcr.classic.Battle:points", a)
{
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);

    game::config::HostConfiguration config;
    config[config.NumExperienceLevels].set(3);

    afl::string::NullTranslator tx;

    // Configure from pcc-v2/tests/vcr/vcr2.dat #1
    game::vcr::classic::Battle t(makeLeftShip(), makeRightShip(), 42, 0);
    t.setType(game::vcr::classic::PHost4, 0);
    t.prepareResult(config, shipList, game::vcr::Battle::NeedCompleteResult);
    a.checkEqual("01. getResultSummary", t.getResultSummary(2, config, shipList, util::NumberFormatter(false, false), tx), "They have captured our ship (2 BP, 5 EP).");
    a.checkEqual("02. getResultSummary", t.getResultSummary(3, config, shipList, util::NumberFormatter(false, false), tx), "We captured their ship (2 BP, 5 EP).");

    // Points for each side
    {
        game::vcr::Score s;
        a.checkEqual("11. computeScores", t.computeScores(s, 0, config, shipList), true);
        a.checkEqual("12. getBuildMillipoints", s.getBuildMillipoints().min(), 0);
        a.checkEqual("13. getBuildMillipoints", s.getBuildMillipoints().max(), 0);
        a.checkEqual("14. getExperience",       s.getExperience().min(), 0);
        a.checkEqual("15. getExperience",       s.getExperience().max(), 0);
        a.checkEqual("16. getTonsDestroyed",    s.getTonsDestroyed().min(), 0);
        a.checkEqual("17. getTonsDestroyed",    s.getTonsDestroyed().max(), 0);
    }
    {
        game::vcr::Score s;
        a.checkEqual("18. computeScores", t.computeScores(s, 1, config, shipList), true);
        a.checkEqual("19. getBuildMillipoints", s.getBuildMillipoints().min(), 2200);
        a.checkEqual("20. getBuildMillipoints", s.getBuildMillipoints().max(), 2200);
        a.checkEqual("21. getExperience",       s.getExperience().min(), 5);
        a.checkEqual("22. getExperience",       s.getExperience().max(), 5);
        a.checkEqual("23. getTonsDestroyed",    s.getTonsDestroyed().min(), 0);
        a.checkEqual("24. getTonsDestroyed",    s.getTonsDestroyed().max(), 0);
    }
}

AFL_TEST("game.vcr.classic.Battle:points:range", a)
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
        game::vcr::classic::Battle t(makeLeftShip(), makeRightShip(), 42, 0);
        t.setType(game::vcr::classic::PHost4, 0);
        t.prepareResult(config, shipList, game::vcr::Battle::NeedCompleteResult);
        a.checkEqual("01", t.getResultSummary(3, config, shipList, fmt, tx), "We captured their ship (4 ... 19 BP, 5 EP).");
    }

    // We know that captor is aggressor
    {
        game::vcr::Object obj(makeRightShip());
        obj.setRole(game::vcr::Object::AggressorRole);
        game::vcr::classic::Battle t(makeLeftShip(), obj, 42, 0);
        t.setType(game::vcr::classic::PHost4, 0);
        t.prepareResult(config, shipList, game::vcr::Battle::NeedCompleteResult);
        a.checkEqual("11", t.getResultSummary(3, config, shipList, fmt, tx), "We captured their ship (19 BP, 5 EP).");
    }

    // We know that captor is opponent
    {
        game::vcr::Object obj(makeRightShip());
        obj.setRole(game::vcr::Object::OpponentRole);
        game::vcr::classic::Battle t(makeLeftShip(), obj, 42, 0);
        t.setType(game::vcr::classic::PHost4, 0);
        t.prepareResult(config, shipList, game::vcr::Battle::NeedCompleteResult);
        a.checkEqual("21", t.getResultSummary(3, config, shipList, fmt, tx), "We captured their ship (4 BP, 5 EP).");
    }
}
