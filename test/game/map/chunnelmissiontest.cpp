/**
  *  \file test/game/map/chunnelmissiontest.cpp
  *  \brief Test for game::map::ChunnelMission
  */

#include "game/map/chunnelmission.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/root.hpp"
#include "game/test/simpleturn.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"

using game::spec::BasicHullFunction;

namespace {
    void checkRange(afl::test::Assert a, game::HostVersion host, int dx, int dy, int fuel, bool expectSetup, bool expectMission)
    {
        String_t name = afl::string::Format("host=%s, d=(%d,%d), fuel=%d", host.toString(), dx, dy, fuel);

        game::test::SimpleTurn t;   // univ, config, shipList, version
        game::TeamSettings teams;
        game::UnitScoreDefinitionList shipScores;

        // Initiator
        game::map::Point initPos(500, 500);
        game::map::Ship& init = t.addShip(32, 6, game::map::Object::Playable);
        init.setFriendlyCode(String_t("foo"));
        init.addShipSpecialFunction(t.shipList().modifiedHullFunctions().getFunctionIdFromHostId(BasicHullFunction::FirecloudChunnel));
        init.setCargo(game::Element::Neutronium, fuel);
        init.setPosition(initPos);
        init.setWaypoint(initPos);

        // Mate
        game::map::Point matePos(500 + dx, 500 + dy);
        game::map::Ship& mate = t.addShip(77, 6, game::map::Object::Playable);
        mate.setFriendlyCode(String_t("bar"));
        mate.addShipSpecialFunction(t.shipList().modifiedHullFunctions().getFunctionIdFromHostId(BasicHullFunction::FirecloudChunnel));
        mate.setCargo(game::Element::Neutronium, 100);
        mate.setPosition(matePos);
        mate.setWaypoint(matePos);

        // Root
        afl::base::Ref<game::Root> root(game::test::makeRoot(host));

        // Can we set up a chunnel?
        bool valid = game::map::isValidChunnelMate(init, mate, t.mapConfiguration(), *root, shipScores, teams, t.shipList());
        a(name).checkEqual("01. valid", valid, expectSetup);

        // Set up and parse
        init.setFriendlyCode(String_t("077"));
        game::map::ChunnelMission msn;
        bool msnOK = msn.check(init, t.universe(), t.mapConfiguration(), shipScores, teams, t.shipList(), *root);

        // Verify parse
        a(name).checkEqual("11. msnOK", msnOK, true);
        a(name).checkEqual("12. getTargetId", msn.getTargetId(), 77);

        // Verify consistency
        if (expectMission) {
            a(name).checkEqual("13. getFailureReasons", msn.getFailureReasons(), 0);
        } else {
            a(name).checkDifferent("14. getFailureReasons", msn.getFailureReasons(), 0);
        }
    }

    void checkAbilities(afl::test::Assert a, int initFunction, int otherInitFunction, int mateFunction, bool expectSuccess, int expectKind)
    {
        String_t name = afl::string::Format("init=%d, mate=%d", initFunction, mateFunction);

        game::test::SimpleTurn t;   // univ, config, shipList, version
        game::UnitScoreDefinitionList shipScores;
        game::TeamSettings teams;

        // Initiator
        game::map::Point initPos(500, 500);
        game::map::Ship& init = t.addShip(55, 6, game::map::Object::Playable);
        init.setFriendlyCode(String_t("444"));
        init.addShipSpecialFunction(t.shipList().modifiedHullFunctions().getFunctionIdFromHostId(initFunction));
        if (otherInitFunction != 0) {
            init.addShipSpecialFunction(t.shipList().modifiedHullFunctions().getFunctionIdFromHostId(otherInitFunction));
        }
        init.setCargo(game::Element::Neutronium, 100);
        init.setPosition(initPos);
        init.setWaypoint(initPos);

        // Mate
        game::map::Point matePos(600, 600);
        game::map::Ship& mate = t.addShip(444, 6, game::map::Object::Playable);
        mate.setFriendlyCode(String_t("bar"));
        mate.addShipSpecialFunction(t.shipList().modifiedHullFunctions().getFunctionIdFromHostId(mateFunction));
        mate.setCargo(game::Element::Neutronium, 100);
        mate.setPosition(matePos);
        mate.setWaypoint(matePos);

        // Root
        afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))));

        // Would we be able to set up a chunnel?
        {
            bool valid = game::map::isValidChunnelMate(init, mate, t.mapConfiguration(), *root, shipScores, teams, t.shipList());
            a(name).checkEqual("01. valid", valid, expectSuccess);
        }

        // Do we recognize the chunnel?
        {
            game::map::ChunnelMission msn;
            bool valid = msn.check(init, t.universe(), t.mapConfiguration(), shipScores, teams, t.shipList(), *root);
            a(name).checkEqual("11. check", valid, expectSuccess);
            a(name).checkEqual("12. isValid", msn.isValid(), expectSuccess);
            if (expectSuccess) {
                a(name).checkEqual("13. getTargetId", msn.getTargetId(), 444);
                a(name).checkEqual("14. getChunnelType", msn.getChunnelType(), expectKind);
                a(name).checkEqual("15. getFailureReasons", msn.getFailureReasons(), 0);
            }
        }
    }
}

/** Test range behaviour for PHost.
    This test is similar to c2hosttest/ship/01_chunnel for PHost. */
AFL_TEST("game.map.ChunnelMission:ranges:PHost", a)
{
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(3, 0, 0));

    // Always fails with 50 fuel, succeed starting at dx=15 with 51 fuel, always fail at 9 ly.
    for (int dx = 0; dx < 30; ++dx) {
        checkRange(a, host, 99, dx, 50, dx >= 15, false);
        checkRange(a, host, 99, dx, 51, dx >= 15, dx >= 15);
        checkRange(a, host,  9, dx, 51, false,    false);
    }
}

/** Test range behaviour for Host.
    This test is similar to c2hosttest/ship/01_chunnel for Host. */
AFL_TEST("game.map.ChunnelMission:ranges:Host", a)
{
    game::HostVersion host(game::HostVersion::Host, MKVERSION(3, 22, 40));

    // Succeed starting with dx=10, even with just 50 fuel, always fail at 9 ly.
    for (int dx = 0; dx < 30; ++dx) {
        checkRange(a, host, 99, dx, 50, dx >= 10, dx >= 10);
        checkRange(a, host, 99, dx, 51, dx >= 10, dx >= 10);
        checkRange(a, host,  9, dx, 51, false,    false);
    }
}

// Additional possible test: THost 3.20 .. 3.22.25 succeeds (host, 9, dx, 51, dx >= 5) and all the others
// Additional possible test: older THost does not have chunnel (neither does PHost 2 probably)

/** Test consistent handling of abilities. */
AFL_TEST("game.map.ChunnelMission:abilities", a)
{
    // FirecloudChunnel
    using game::map::ChunnelMission;
    const int chk_All = ChunnelMission::chk_Self + ChunnelMission::chk_Others;

    checkAbilities(a, BasicHullFunction::FirecloudChunnel, 0, BasicHullFunction::FirecloudChunnel, true,  chk_All);
    checkAbilities(a, BasicHullFunction::FirecloudChunnel, 0, BasicHullFunction::ChunnelTarget,    true,  chk_All);
    checkAbilities(a, BasicHullFunction::FirecloudChunnel, 0, BasicHullFunction::ChunnelSelf,      false, 0);
    checkAbilities(a, BasicHullFunction::FirecloudChunnel, 0, BasicHullFunction::ChunnelOthers,    false, 0);

    checkAbilities(a, BasicHullFunction::ChunnelSelf, 0, BasicHullFunction::FirecloudChunnel, true,  ChunnelMission::chk_Self);
    checkAbilities(a, BasicHullFunction::ChunnelSelf, 0, BasicHullFunction::ChunnelTarget,    true,  ChunnelMission::chk_Self);
    checkAbilities(a, BasicHullFunction::ChunnelSelf, 0, BasicHullFunction::ChunnelSelf,      false, 0);
    checkAbilities(a, BasicHullFunction::ChunnelSelf, 0, BasicHullFunction::ChunnelOthers,    false, 0);

    checkAbilities(a, BasicHullFunction::ChunnelOthers, 0, BasicHullFunction::FirecloudChunnel, true,  ChunnelMission::chk_Others);
    checkAbilities(a, BasicHullFunction::ChunnelOthers, 0, BasicHullFunction::ChunnelTarget,    true,  ChunnelMission::chk_Others);
    checkAbilities(a, BasicHullFunction::ChunnelOthers, 0, BasicHullFunction::ChunnelSelf,      false, 0);
    checkAbilities(a, BasicHullFunction::ChunnelOthers, 0, BasicHullFunction::ChunnelOthers,    false, 0);

    checkAbilities(a, BasicHullFunction::ChunnelTarget, 0, BasicHullFunction::FirecloudChunnel, false, 0);
    checkAbilities(a, BasicHullFunction::ChunnelTarget, 0, BasicHullFunction::ChunnelTarget,    false, 0);
    checkAbilities(a, BasicHullFunction::ChunnelTarget, 0, BasicHullFunction::ChunnelSelf,      false, 0);
    checkAbilities(a, BasicHullFunction::ChunnelTarget, 0, BasicHullFunction::ChunnelOthers,    false, 0);
}

/** Test consistent handling of combination abilities. */
AFL_TEST("game.map.ChunnelMission:ability-combination", a)
{
    using game::map::ChunnelMission;
    const int chk_All = ChunnelMission::chk_Self + ChunnelMission::chk_Others;

    // Chunneling a ship that has ChunnelSelf + ChunnelOthers will produce chk_All
    checkAbilities(a, BasicHullFunction::ChunnelSelf, BasicHullFunction::ChunnelOthers, BasicHullFunction::ChunnelTarget, true,  chk_All);
}

/** Test alliance handling. */
AFL_TEST("game.map.ChunnelMission:alliances", a)
{
    const int INIT_OWNER = 6;
    const int MATE_OWNER = 7;

    game::test::SimpleTurn t;   // univ, config, shipList, version
    game::UnitScoreDefinitionList shipScores;

    // Initiator
    game::map::Point initPos(500, 500);
    game::map::Ship& init = t.addShip(55, INIT_OWNER, game::map::Object::Playable);
    init.setFriendlyCode(String_t("444"));
    init.addShipSpecialFunction(t.shipList().modifiedHullFunctions().getFunctionIdFromHostId(BasicHullFunction::FirecloudChunnel));
    init.setCargo(game::Element::Neutronium, 100);
    init.setPosition(initPos);
    init.setWaypoint(initPos);

    // Mate
    // Create as INIT_OWNER and change to MATE_OWNER so it has the correct source mask (for isReliablyVisible()).
    game::map::Point matePos(600, 600);
    game::map::Ship& mate = t.addShip(444, INIT_OWNER, game::map::Object::Playable);
    mate.setOwner(MATE_OWNER);
    mate.setFriendlyCode(String_t("bar"));
    mate.addShipSpecialFunction(t.shipList().modifiedHullFunctions().getFunctionIdFromHostId(BasicHullFunction::FirecloudChunnel));
    mate.setCargo(game::Element::Neutronium, 100);
    mate.setPosition(matePos);
    mate.setWaypoint(matePos);

    // Root/HostConfiguration for both cases
    afl::base::Ref<game::Root> rootEnabled(game::test::makeRoot(game::HostVersion()));
    rootEnabled->hostConfiguration()[game::config::HostConfiguration::AllowAlliedChunneling].set(1);

    afl::base::Ref<game::Root> rootDisabled(game::test::makeRoot(game::HostVersion()));
    rootDisabled->hostConfiguration()[game::config::HostConfiguration::AllowAlliedChunneling].set(0);

    // TeamSettings for both cases
    game::TeamSettings teamAllied;
    teamAllied.setPlayerTeam(INIT_OWNER, INIT_OWNER);
    teamAllied.setPlayerTeam(MATE_OWNER, INIT_OWNER);

    game::TeamSettings teamDefault;
    teamDefault.setPlayerTeam(INIT_OWNER, INIT_OWNER);
    teamDefault.setPlayerTeam(MATE_OWNER, MATE_OWNER);

    // Default case
    {
        a.check("01. default", !game::map::isValidChunnelMate(init, mate, t.mapConfiguration(), *rootDisabled, shipScores, teamDefault, t.shipList()));
    }

    // Allied, but team chunnel disabled
    {
        a.check("11. allied, team off", !game::map::isValidChunnelMate(init, mate, t.mapConfiguration(), *rootDisabled, shipScores, teamAllied, t.shipList()));
    }

    // No allied, but team chunnel enabled
    {
        a.check("21. not allied, team off", !game::map::isValidChunnelMate(init, mate, t.mapConfiguration(), *rootEnabled, shipScores, teamDefault, t.shipList()));
    }

    // Allied and team chunnel enabled
    {
        a.check("31. allied, team on", game::map::isValidChunnelMate(init, mate, t.mapConfiguration(), *rootEnabled, shipScores, teamAllied, t.shipList()));
    }
}
