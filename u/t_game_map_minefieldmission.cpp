/**
  *  \file u/t_game_map_minefieldmission.cpp
  *  \brief Test for game::map::MinefieldMission
  */

#include "game/map/minefieldmission.hpp"

#include "t_game_map.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/root.hpp"
#include "game/test/simpleturn.hpp"
#include "game/unitscoredefinitionlist.hpp"

using game::HostVersion;
using game::RegistrationKey;
using game::config::HostConfiguration;
using game::map::Minefield;
using game::map::MinefieldMission;
using game::map::Point;
using game::map::Ship;

namespace {
    struct FriendlyCodeTestCase {
        char friendlyCode[4];
        int expected;
    };

    struct TestHarness {
        game::test::SimpleTurn turn;
        game::test::RegistrationKey key;
        game::UnitScoreDefinitionList shipScores;

        TestHarness(RegistrationKey::Status st = RegistrationKey::Unregistered)
            : turn(),
              key(st, 7)
            { }
    };

    Ship& addFreighter(TestHarness& h, game::Id_t id, int owner)
    {
        Ship& sh = h.turn.addShip(id, owner, Ship::Playable);
        sh.setNumLaunchers(0);
        sh.setTorpedoType(0);
        sh.setAmmo(0);
        return sh;
    }

    Ship& addTorper(TestHarness& h, game::Id_t id, int owner)
    {
        Ship& sh = h.turn.addShip(id, owner, Ship::Playable);
        sh.setNumLaunchers(1);
        sh.setTorpedoType(7);
        sh.setAmmo(60);
        return sh;
    }

    void addMinefield(TestHarness& h, game::Id_t id, Point pos, int32_t units, int owner, bool isWeb)
    {
        game::map::Minefield* mf = h.turn.universe().minefields().create(id);
        TS_ASSERT(mf);
        mf->addReport(pos, owner, isWeb ? Minefield::IsWeb : Minefield::IsMine, Minefield::UnitsKnown, units, 1, Minefield::MinefieldScanned);
        mf->internalCheck(1, h.turn.version(), h.turn.config());
    }
}

/** Test initial state.
    A: create MinefieldMission object. Check initial state.
    E: all attributes at defaults. */
void
TestGameMapMinefieldMission::testInit()
{
    MinefieldMission testee;
    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 0);
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 0);
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 0);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 0);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), false);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), false);
}

/** Test mine laying with an empty ship.
    This verifies that we can deal with unknown data.
    A: call checkLayMission with a default-initialized ship.
    E: must report false. */
void
TestGameMapMinefieldMission::testLayEmptyShip()
{
    MinefieldMission testee;
    Ship ship(77);
    game::map::Universe univ;
    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)));
    game::UnitScoreDefinitionList shipScores;     // required for hull functions, which are required to determine FCode availability
    game::spec::ShipList shipList;                // required for fcodes and hull functions

    TS_ASSERT_EQUALS(testee.checkLayMission(ship, univ, *root, game::map::Configuration(), shipScores, shipList), false);
}

/** Test mine laying with a freighter.
    A: call checkLayMission with a freighter.
    E: must report false. */
void
TestGameMapMinefieldMission::testLayFreighter()
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addFreighter(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t(""));
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), false);
}

/** Test mine laying with wrong mission.
    A: call checkLayMission with a torper that has mission Explore.
    E: must report false. */
void
TestGameMapMinefieldMission::testLayOther()
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(1, 0, 0);
    sh.setFriendlyCode(String_t(""));
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), false);
}

/** Test mine laying (successful base case).
    A: call checkLayMission with a torper that has mission Lay Mines.
    E: must report new minefield being laid. */
void
TestGameMapMinefieldMission::testLayNormal()
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t(""));
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 0);
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 3);
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 60);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 60*49);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), false);
}

/** Test mine laying disabled in config.
    A: set AllowMinefields=No. Call checkLayMission with a torper that has mission Lay Mines.
    E: must report false. */
void
TestGameMapMinefieldMission::testLayNormalDisabled()
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t(""));
    h.turn.config()[HostConfiguration::AllowMinefields].set(false);
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), false);
}

/** Test mine laying as robots.
    This verifies that UnitsPerTorpRate is correctly handled.
    A: call checkLayMission with a Robotic torper that has mission Lay Mines.
    E: must report new minefield being laid with large rate. */
void
TestGameMapMinefieldMission::testLayRobot()
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 9);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t(""));
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 0);
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 9);
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 60);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 4*60*49); /* 4x bonus applied */
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), false);
}

/** Test mine laying using "mdX" friendly codes.
    A: call checkLayMission with a torper that has mission Lay Mines and an "mdX" friendly code.
    E: must report new minefield being laid with correct number of torps. */
void
TestGameMapMinefieldMission::testLayDropFCode()
{
    static const FriendlyCodeTestCase TESTCASES[] = {
        { "mdh",  30 },
        { "mdq",  15 },
        { "md1",  10 },
        { "md5",  50 },
        { "md9",  60 },
        { "md0",  60 },
    };

    for (size_t i = 0; i < countof(TESTCASES); ++i) {
        const FriendlyCodeTestCase& c = TESTCASES[i];

        MinefieldMission testee;
        TestHarness h;

        Ship& sh = addTorper(h, 222, 3);
        sh.setMission(3, 0, 0);
        sh.setFriendlyCode(String_t(c.friendlyCode));
        TSM_ASSERT_EQUALS(c.friendlyCode, testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

        TSM_ASSERT_EQUALS(c.friendlyCode, testee.getRequiredMinefieldId(), 0);
        TSM_ASSERT_EQUALS(c.friendlyCode, testee.getMinefieldOwner(), 3);
        TSM_ASSERT_EQUALS(c.friendlyCode, testee.isWeb(), false);
        TSM_ASSERT_EQUALS(c.friendlyCode, testee.getNumTorpedoes(), c.expected);
        TSM_ASSERT_EQUALS(c.friendlyCode, testee.getNumUnits(), c.expected*49);
        TSM_ASSERT_EQUALS(c.friendlyCode, testee.isMissionUsed(), true);
        TSM_ASSERT_EQUALS(c.friendlyCode, testee.isFriendlyCodeUsed(), true);
    }
}

/** Test mine laying using disallowed "mdX" friendly code.
    A: Set friendly-code "mdh" to registered-only, but don't add a key. Call checkLayMission on ship with "mdh" fcode.
    E: must report minefield laid with all torps (fcode ignored). */
void
TestGameMapMinefieldMission::testLayDropFCodeDisallowed()
{
    MinefieldMission testee;
    TestHarness h;
    afl::string::NullTranslator tx;

    h.turn.shipList().friendlyCodes().addCode(game::spec::FriendlyCode("mdh", "rs,drop half", tx));

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t("mdh"));
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 0);
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 3);
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 60);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 60*49);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), false);
}

/** Test mine laying using inapplicable "mdX" friendly code.
    A: Set friendly-code "mdh" to planets-only. Call checkLayMission on ship with "mdh" fcode.
    E: must report minefield laid with all torps (fcode ignored). */
void
TestGameMapMinefieldMission::testLayDropFCodeInapplicable()
{
    MinefieldMission testee;
    TestHarness h;
    afl::string::NullTranslator tx;

    h.turn.shipList().friendlyCodes().addCode(game::spec::FriendlyCode("mdh", "p,drop half", tx));

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t("mdh"));
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 0);
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 3);
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 60);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 60*49);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), false);
}

/** Test mine laying using "miX" friendly code.
    A: Call checkLayMission on ship with "miX" fcode.
    E: must report minefield laid with changed owner. */
void
TestGameMapMinefieldMission::testLayIdentityFCode()
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t("mi4"));
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 0);
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 4);
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 60);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 60*49);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), true);
}

/** Test mine laying using "miX" friendly code, Robot case.
    A: Call checkLayMission on Robotic ship with "miX" fcode.
    E: must report minefield laid with changed owner, 4x bonus not applied. */
void
TestGameMapMinefieldMission::testLayIdentityFCodeRobot()
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 9);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t("mi4"));
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 0);
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 4);
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 60);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 60*49);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), true);
}

/** Test laying web mines.
    A: Call checkLayMission on Tholian ship with mission 9.
    E: must report web field laid. */
void
TestGameMapMinefieldMission::testLayWeb()
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 7);
    sh.setMission(9, 0, 0);
    sh.setFriendlyCode(String_t(""));
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 0);
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 7);
    TS_ASSERT_EQUALS(testee.isWeb(), true);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 60);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 60*49);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), false);
}

/** Test laying web mines disabled in config.
    A: Set AllowWebMines=No. Call checkLayMission on Tholian ship with mission 9.
    E: must report false. */
void
TestGameMapMinefieldMission::testLayWebDisabled()
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 9);
    sh.setMission(9, 0, 0);
    sh.setFriendlyCode(String_t(""));
    h.turn.config()[HostConfiguration::AllowWebMines].set(false);
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), false);
}

/** Test laying web mines, other race.
    A: Call checkLayMission on non-Tholian ship with mission 9.
    E: must report false. */
void
TestGameMapMinefieldMission::testLayWebWrongRace()
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 4);
    sh.setMission(9, 0, 0);
    sh.setFriendlyCode(String_t(""));
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), false);
}

/** Test laying minefield using "Lay Mines" extended mission.
    A: Call checkLayMission on ship with mission "Lay Mines" and parameters.
    E: must report new minefield being laid with given parameters. */
void
TestGameMapMinefieldMission::testLayExtended()
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(21, 12, 4);                    /* pmsn_LayMines + default ExtMissionsStartAt */
    sh.setFriendlyCode(String_t("mi5"));         /* not relevant here */
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 0);
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 4); /* from mission */
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 12);  /* from mission */
    TS_ASSERT_EQUALS(testee.getNumUnits(), 12*49);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), false);
}

/** Test laying minefield using "Lay Web Mines" extended mission.
    A: Call checkLayMission on ship with mission "Lay Web Mines" and parameters.
    E: must report new web field being laid with given parameters. */
void
TestGameMapMinefieldMission::testLayWebExtended()
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    Ship& sh = addTorper(h, 222, 7);
    sh.setMission(22, 12, 9);                    /* pmsn_LayWeb + default ExtMissionsStartAt */
    sh.setFriendlyCode(String_t("mi5"));         /* not relevant here */
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 0);
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 9); /* from mission */
    TS_ASSERT_EQUALS(testee.isWeb(), true);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 12);  /* from mission */
    TS_ASSERT_EQUALS(testee.getNumUnits(), 12*49);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), false);
}

/** Test laying minefield using "Add Mines To" extended mission.
    A: Call checkLayMission on ship with mission "Add Mines To" and parameters.
    E: must report new minefield being laid with given parameters. */
void
TestGameMapMinefieldMission::testLayInExtended()
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(36, 17, 0);                    /* pmsn_LayMinesIn + default ExtMissionsStartAt */
    sh.setFriendlyCode(String_t("mi5"));
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 0);
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 5);
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 17);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 17*49);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), true);
}

/** Test laying minefield using "Add Web Mines To" extended mission.
    A: Call checkLayMission on ship with mission "Add Web Mines To" and parameters.
    E: must report new web field being laid with given parameters. */
void
TestGameMapMinefieldMission::testLayWebInExtended()
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    Ship& sh = addTorper(h, 222, 7);
    sh.setMission(37, 17, 0);                    /* pmsn_LayWWebIn + default ExtMissionsStartAt */
    sh.setFriendlyCode(String_t("md3"));         /* not relevant */
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 0);
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 7);
    TS_ASSERT_EQUALS(testee.isWeb(), true);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 17);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 17*49);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), false);
}

/** Test extending a minefield, Host case.
    A: Create minefields. Use Tim-Host. Call checkLayMission on ship with mission "Lay Mines".
    E: must report closest minefield being extended. */
void
TestGameMapMinefieldMission::testLayExtendHost()
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    h.turn.version() = HostVersion(HostVersion::Host, MKVERSION(3,22,40));

    // Ship at (1000,1000)
    h.turn.setPosition(Point(1000, 1000));
    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t(""));

    // Some minefields
    addMinefield(h, 10, Point(1010, 1000), 2000, 3, false);
    addMinefield(h, 20, Point(1020, 1000), 2000, 3, false);
    addMinefield(h, 30, Point(1005, 1000), 2000, 3, false);
    addMinefield(h, 40, Point(1030, 1000), 2000, 3, false);

    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 30); // closest
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 3);
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 60);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 60*49);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), false);
}

/** Test extending a minefield, Host failure case.
    A: Create minefields, closest does not overlap ship. Use Tim-Host. Call checkLayMission on ship with mission "Lay Mines".
    E: must report new minefield being laid. */
void
TestGameMapMinefieldMission::testLayExtendHostFail()
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    h.turn.version() = HostVersion(HostVersion::Host, MKVERSION(3,22,40));

    // Ship at (1000,1000)
    h.turn.setPosition(Point(1000, 1000));
    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t(""));

    // Some minefields
    addMinefield(h, 10, Point(1010, 1000), 2000, 3, false);
    addMinefield(h, 20, Point(1020, 1000), 2000, 3, false);
    addMinefield(h, 30, Point(1005, 1000), 20, 3, false);
    addMinefield(h, 40, Point(1030, 1000), 2000, 3, false);

    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 0); // make new field
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 3);
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 60);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 60*49);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), false);
}

/** Test extending a minefield, PHost case.
    A: Create minefields. Use PHost. Call checkLayMission on ship with mission "Lay Mines".
    E: must report lowest-Id minefield being extended. */
void
TestGameMapMinefieldMission::testLayExtendPHost()
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    // Ship at (1000,1000)
    h.turn.setPosition(Point(1000, 1000));
    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t(""));

    // Some minefields
    addMinefield(h, 10, Point(1010, 1000), 2000, 3, false);
    addMinefield(h, 20, Point(1020, 1000), 2000, 3, false);
    addMinefield(h, 30, Point(1005, 1000), 2000, 3, false);
    addMinefield(h, 40, Point(1030, 1000), 2000, 3, false);

    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 10); // first matching
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 3);
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 60);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 60*49);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), false);
}

/** Test extending a minefield, PHost with extended mission.
    A: Create minefields. Use PHost. Call checkLayMission on ship with mission "Add Mines To" and explicitly given Id.
    E: must report selected Id being extended. */
void
TestGameMapMinefieldMission::testLayExtendId()
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    // Ship at (1000,1000)
    h.turn.setPosition(Point(1000, 1000));
    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(36, 0, 20);                   /* pmsn_LayMinesIn + default ExtMissionsStartAt */
    sh.setFriendlyCode(String_t(""));

    // Some minefields
    addMinefield(h, 10, Point(1010, 1000), 2000, 3, false);
    addMinefield(h, 20, Point(1020, 1000), 2000, 3, false);
    addMinefield(h, 30, Point(1005, 1000), 2000, 3, false);
    addMinefield(h, 40, Point(1030, 1000), 2000, 3, false);

    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 20); // selected
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 3);
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 60);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 60*49);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), false);
}

/** Test extending a minefield, PHost with extended mission, failure case.
    A: Call checkLayMission on ship with mission "Add Mines To" and given Id of non-existing field.
    E: must report false. */
void
TestGameMapMinefieldMission::testLayExtendIdMissing()
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(36, 17, 444);                   /* pmsn_LayMinesIn + default ExtMissionsStartAt */
    sh.setFriendlyCode(String_t(""));
    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), false);
}

/** Test extending a minefield, PHost with extended mission, failure case.
    A: Create minefield not overlapping the ship. Call checkLayMission on ship with mission "Add Mines To" and given Id of that field.
    E: must report false. */
void
TestGameMapMinefieldMission::testLayExtendIdMismatch()
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    // Ship at (1000,1000)
    h.turn.setPosition(Point(1000, 1000));
    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(36, 0, 20);                   /* pmsn_LayMinesIn + default ExtMissionsStartAt */
    sh.setFriendlyCode(String_t(""));

    // Far-away minefield
    addMinefield(h, 20, Point(1500, 1000), 20, 3, false);

    TS_ASSERT_EQUALS(testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), false);
}

/** Test mine scooping with an empty ship.
    This verifies that we can deal with unknown data.
    A: call checkScoopMission with a default-initialized ship.
    E: must report false. */
void
TestGameMapMinefieldMission::testScoopEmpty()
{
    MinefieldMission testee;
    Ship ship(77);
    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)));
    game::UnitScoreDefinitionList shipScores;     // required for hull functions, which are required to determine FCode availability
    game::spec::ShipList shipList;                // required for fcodes and hull functions

    TS_ASSERT_EQUALS(testee.checkScoopMission(ship, *root, shipScores, shipList), false);
}

/** Test mine scooping with a freighter.
    A: call checkScoopMission with a freighter.
    E: must report false. */
void
TestGameMapMinefieldMission::testScoopFreighter()
{
    MinefieldMission testee;

    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)), RegistrationKey::Registered);
    TestHarness h;
    Ship& ship = addFreighter(h, 222, 3);
    ship.setMission(2, 0, 0);
    ship.setFriendlyCode(String_t("msc"));

    TS_ASSERT_EQUALS(testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), false);
}

/** Test mine scooping (successful base case).
    A: call checkScoopMission on a ship with torps, beams, and "msc".
    E: must report success. */
void
TestGameMapMinefieldMission::testScoopFCode()
{
    MinefieldMission testee;

    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)));
    TestHarness h;
    Ship& ship = addTorper(h, 222, 3);
    ship.setNumBeams(2);
    ship.setBeamType(5);
    ship.setMission(2, 0, 0);
    ship.setFriendlyCode(String_t("msc"));

    TS_ASSERT_EQUALS(testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 0);
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 3);
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 0);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 0);
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), true);
}

/** Test mine scooping, inapplicable friendly code.
    A: Define "msc" as planet-only fcode. Call checkScoopMission on a ship with torps, beams, and "msc".
    E: must report false. */
void
TestGameMapMinefieldMission::testScoopFCodeDisabled()
{
    MinefieldMission testee;
    afl::string::NullTranslator tx;

    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)));
    TestHarness h;
    Ship& ship = addTorper(h, 222, 3);
    ship.setNumBeams(2);
    ship.setBeamType(5);
    ship.setMission(2, 0, 0);
    ship.setFriendlyCode(String_t("msc"));

    h.turn.shipList().friendlyCodes().addCode(game::spec::FriendlyCode("msc", "p,foo", tx));

    TS_ASSERT_EQUALS(testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), false);
}

/** Test mine scooping, unusable friendly code.
    A: Define "msc" as registered-only fcode. Add unregistered key. Call checkScoopMission on a ship with torps, beams, and "msc".
    E: must report false. */
void
TestGameMapMinefieldMission::testScoopFCodeUnregistered()
{
    MinefieldMission testee;
    afl::string::NullTranslator tx;

    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)), RegistrationKey::Unregistered);
    TestHarness h;
    Ship& ship = addTorper(h, 222, 3);
    ship.setNumBeams(2);
    ship.setBeamType(5);
    ship.setMission(2, 0, 0);
    ship.setFriendlyCode(String_t("msc"));

    h.turn.shipList().friendlyCodes().addCode(game::spec::FriendlyCode("msc", "sr,foo", tx));

    TS_ASSERT_EQUALS(testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), false);
}

/** Test mine scooping, inapplicable friendly code.
    A: Use Host. Call checkScoopMission on a ship with torps, no beams, and "msc".
    E: must report success. */
void
TestGameMapMinefieldMission::testScoopFCodeNoBeamsHost()
{
    MinefieldMission testee;

    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::Host, MKVERSION(3,22,40)));
    TestHarness h;
    Ship& ship = addTorper(h, 222, 3);
    ship.setNumBeams(0);
    ship.setBeamType(0);
    ship.setMission(2, 0, 0);
    ship.setFriendlyCode(String_t("msc"));

    TS_ASSERT_EQUALS(testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), true);
}

/** Test mine scooping, inapplicable friendly code.
    A: Use PHost. Call checkScoopMission on a ship with torps, no beams, and "msc".
    E: must report false. */
void
TestGameMapMinefieldMission::testScoopFCodeNoBeamsPHost()
{
    MinefieldMission testee;

    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,22,40)));
    TestHarness h;
    Ship& ship = addTorper(h, 222, 3);
    ship.setNumBeams(0);
    ship.setBeamType(0);
    ship.setMission(2, 0, 0);
    ship.setFriendlyCode(String_t("msc"));

    TS_ASSERT_EQUALS(testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), false);
}

/** Test mine scooping using mission.
    A: Use PHost and registered key. Call checkScoopMission on a ship with "Scoop Torpedoes" mission.
    E: must report success. */
void
TestGameMapMinefieldMission::testScoopMission()
{
    MinefieldMission testee;

    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)), RegistrationKey::Registered);
    TestHarness h;
    Ship& ship = addTorper(h, 222, 3);
    ship.setNumBeams(2);
    ship.setBeamType(5);
    ship.setMission(23, 27, 456);                 /* ExtMissionsStartAt + pmsn_ScoopTorps */
    ship.setFriendlyCode(String_t(""));

    TS_ASSERT_EQUALS(testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), true);

    TS_ASSERT_EQUALS(testee.getRequiredMinefieldId(), 456);
    TS_ASSERT_EQUALS(testee.getMinefieldOwner(), 3);
    TS_ASSERT_EQUALS(testee.isWeb(), false);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 27);
    TS_ASSERT_EQUALS(testee.getNumUnits(), 0);    /* not relevant for scooping */
    TS_ASSERT_EQUALS(testee.isMissionUsed(), true);
    TS_ASSERT_EQUALS(testee.isFriendlyCodeUsed(), false);
}

/** Test mine scooping using mission, unregistered.
    A: Use PHost and unregistered key. Call checkScoopMission on a ship with "Scoop Torpedoes" mission.
    E: must report false. */
void
TestGameMapMinefieldMission::testScoopMissionUnregistered()
{
    MinefieldMission testee;

    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)), RegistrationKey::Unregistered);
    TestHarness h;
    Ship& ship = addTorper(h, 222, 3);
    ship.setNumBeams(2);
    ship.setBeamType(5);
    ship.setMission(23, 27, 456);                 /* ExtMissionsStartAt + pmsn_ScoopTorps */
    ship.setFriendlyCode(String_t(""));

    TS_ASSERT_EQUALS(testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), false);
}

