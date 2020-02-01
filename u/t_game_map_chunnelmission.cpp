/**
  *  \file u/t_game_map_chunnelmission.cpp
  *  \brief Test for game::map::ChunnelMission
  */

#include "game/map/chunnelmission.hpp"

#include "t_game_map.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/simpleturn.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"

using game::spec::HullFunction;

namespace {
    void checkRange(game::HostVersion host, int dx, int dy, int fuel, bool expectSetup, bool expectMission)
    {
        afl::string::NullTranslator tx;
        String_t name = afl::string::Format("host=%s, d=(%d,%d), fuel=%d", host.toString(tx), dx, dy, fuel);

        game::test::SimpleTurn t;   // univ, config, shipList, version
        game::UnitScoreDefinitionList shipScores;

        // Initiator
        game::map::Point initPos(500, 500);
        game::map::Ship& init = t.addShip(32, 6, game::map::Object::Playable);
        init.setFriendlyCode(String_t("foo"));
        init.addShipSpecialFunction(t.shipList().modifiedHullFunctions().getFunctionIdFromHostId(HullFunction::FirecloudChunnel));
        init.setCargo(game::Element::Neutronium, fuel);
        init.setPosition(initPos);
        init.setWaypoint(initPos);

        // Mate
        game::map::Point matePos(500 + dx, 500 + dy);
        game::map::Ship& mate = t.addShip(77, 6, game::map::Object::Playable);
        mate.setFriendlyCode(String_t("bar"));
        mate.addShipSpecialFunction(t.shipList().modifiedHullFunctions().getFunctionIdFromHostId(HullFunction::FirecloudChunnel));
        mate.setCargo(game::Element::Neutronium, 100);
        mate.setPosition(matePos);
        mate.setWaypoint(matePos);

        // Root
        game::Root root(afl::io::InternalDirectory::create("<empty>"),
                        *new game::test::SpecificationLoader(),
                        host,
                        std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Unknown, 6)),
                        std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                        std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                        game::Root::Actions_t());

        // Can we set up a chunnel?
        bool valid = game::map::isValidChunnelMate(init, mate, t.universe().config(), root, shipScores, t.shipList());
        TSM_ASSERT_EQUALS(name.c_str(), valid, expectSetup);

        // Set up and parse
        init.setFriendlyCode(String_t("077"));
        game::map::ChunnelMission msn;
        bool msnOK = msn.check(init, t.universe(), shipScores, t.shipList(), root);

        // Verify parse
        TSM_ASSERT_EQUALS(name.c_str(), msnOK, true);
        TSM_ASSERT_EQUALS(name.c_str(), msn.getTargetId(), 77);

        // Verify consistency
        if (expectMission) {
            TSM_ASSERT_EQUALS(name.c_str(), msn.getFailureReasons(), 0);
        } else {
            TSM_ASSERT_DIFFERS(name.c_str(), msn.getFailureReasons(), 0);
        }
    }

    void checkAbilities(int initFunction, int otherInitFunction, int mateFunction, bool expectSuccess, int expectKind)
    {
        String_t name = afl::string::Format("init=%d, mate=%d", initFunction, mateFunction);

        game::test::SimpleTurn t;   // univ, config, shipList, version
        game::UnitScoreDefinitionList shipScores;

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
        game::Root root(afl::io::InternalDirectory::create("<empty>"),
                        *new game::test::SpecificationLoader(),
                        game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)),
                        std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Unknown, 6)),
                        std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                        std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                        game::Root::Actions_t());

        // Would we be able to set up a chunnel?
        {
            bool valid = game::map::isValidChunnelMate(init, mate, t.universe().config(), root, shipScores, t.shipList());
            TSM_ASSERT_EQUALS(name.c_str(), valid, expectSuccess);
        }

        // Do we recognize the chunnel?
        {
            game::map::ChunnelMission msn;
            bool valid = msn.check(init, t.universe(), shipScores, t.shipList(), root);
            TSM_ASSERT_EQUALS(name.c_str(), valid, expectSuccess);
            TSM_ASSERT_EQUALS(name.c_str(), msn.isValid(), expectSuccess);
            if (expectSuccess) {
                TSM_ASSERT_EQUALS(name.c_str(), msn.getTargetId(), 444);
                TSM_ASSERT_EQUALS(name.c_str(), msn.getChunnelType(), expectKind);
                TSM_ASSERT_EQUALS(name.c_str(), msn.getFailureReasons(), 0);
            }
        }        
    }
}

/** Test range behaviour for PHost.
    This test is similar to c2hosttest/ship/01_chunnel for PHost. */
void
TestGameMapChunnelMission::testRangesPHost()
{
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(3, 0, 0));

    // Always fails with 50 fuel, succeed starting at dx=15 with 51 fuel, always fail at 9 ly.
    for (int dx = 0; dx < 30; ++dx) {
        checkRange(host, 99, dx, 50, dx >= 15, false);
        checkRange(host, 99, dx, 51, dx >= 15, dx >= 15);
        checkRange(host,  9, dx, 51, false,    false);
    }
}

/** Test range behaviour for PHost.
    This test is similar to c2hosttest/ship/01_chunnel for PHost. */
void
TestGameMapChunnelMission::testRangesTHost()
{
    game::HostVersion host(game::HostVersion::Host, MKVERSION(3, 22, 40));

    // Succeed starting with dx=10, even with just 50 fuel, always fail at 9 ly.
    for (int dx = 0; dx < 30; ++dx) {
        checkRange(host, 99, dx, 50, dx >= 10, dx >= 10);
        checkRange(host, 99, dx, 51, dx >= 10, dx >= 10);
        checkRange(host,  9, dx, 51, false,    false);
    }
}

// Additional possible test: THost 3.20 .. 3.22.25 succeeds (host, 9, dx, 51, dx >= 5) and all the others
// Additional possible test: older THost does not have chunnel (neither does PHost 2 probably)

/** Test consistent handling of abilities. */
void
TestGameMapChunnelMission::testAbilities()
{
    // FirecloudChunnel
    using game::map::ChunnelMission;
    const int chk_All = ChunnelMission::chk_Self + ChunnelMission::chk_Others;

    checkAbilities(HullFunction::FirecloudChunnel, 0, HullFunction::FirecloudChunnel, true,  chk_All);
    checkAbilities(HullFunction::FirecloudChunnel, 0, HullFunction::ChunnelTarget,    true,  chk_All);
    checkAbilities(HullFunction::FirecloudChunnel, 0, HullFunction::ChunnelSelf,      false, 0);
    checkAbilities(HullFunction::FirecloudChunnel, 0, HullFunction::ChunnelOthers,    false, 0);

    checkAbilities(HullFunction::ChunnelSelf, 0, HullFunction::FirecloudChunnel, true,  ChunnelMission::chk_Self);
    checkAbilities(HullFunction::ChunnelSelf, 0, HullFunction::ChunnelTarget,    true,  ChunnelMission::chk_Self);
    checkAbilities(HullFunction::ChunnelSelf, 0, HullFunction::ChunnelSelf,      false, 0);
    checkAbilities(HullFunction::ChunnelSelf, 0, HullFunction::ChunnelOthers,    false, 0);

    checkAbilities(HullFunction::ChunnelOthers, 0, HullFunction::FirecloudChunnel, true,  ChunnelMission::chk_Others);
    checkAbilities(HullFunction::ChunnelOthers, 0, HullFunction::ChunnelTarget,    true,  ChunnelMission::chk_Others);
    checkAbilities(HullFunction::ChunnelOthers, 0, HullFunction::ChunnelSelf,      false, 0);
    checkAbilities(HullFunction::ChunnelOthers, 0, HullFunction::ChunnelOthers,    false, 0);

    checkAbilities(HullFunction::ChunnelTarget, 0, HullFunction::FirecloudChunnel, false, 0);
    checkAbilities(HullFunction::ChunnelTarget, 0, HullFunction::ChunnelTarget,    false, 0);
    checkAbilities(HullFunction::ChunnelTarget, 0, HullFunction::ChunnelSelf,      false, 0);
    checkAbilities(HullFunction::ChunnelTarget, 0, HullFunction::ChunnelOthers,    false, 0);
}

/** Test consistent handling of combination abilities. */
void
TestGameMapChunnelMission::testCombinationAbilities()
{
    using game::map::ChunnelMission;
    const int chk_All = ChunnelMission::chk_Self + ChunnelMission::chk_Others;

    // Chunneling a ship that has ChunnelSelf + ChunnelOthers will produce chk_All
    checkAbilities(HullFunction::ChunnelSelf, HullFunction::ChunnelOthers, HullFunction::ChunnelTarget, true,  chk_All);
}

