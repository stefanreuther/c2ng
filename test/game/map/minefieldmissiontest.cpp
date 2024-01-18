/**
  *  \file test/game/map/minefieldmissiontest.cpp
  *  \brief Test for game::map::MinefieldMission
  */

#include "game/map/minefieldmission.hpp"

#include "afl/base/countof.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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
        mf->addReport(pos, owner, isWeb ? Minefield::IsWeb : Minefield::IsMine, Minefield::UnitsKnown, units, 1, Minefield::MinefieldScanned);
        mf->internalCheck(1, h.turn.version(), h.turn.config());
    }
}

/** Test initial state.
    A: create MinefieldMission object. Check initial state.
    E: all attributes at defaults. */
AFL_TEST("game.map.MinefieldMission:init", a)
{
    MinefieldMission testee;
    a.checkEqual("01. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0);
    a.checkEqual("02. getMinefieldOwner", testee.getMinefieldOwner(), 0);
    a.checkEqual("03. isWeb", testee.isWeb(), false);
    a.checkEqual("04. getNumTorpedoes", testee.getNumTorpedoes(), 0);
    a.checkEqual("05. getNumUnits", testee.getNumUnits(), 0);
    a.checkEqual("06. isMissionUsed", testee.isMissionUsed(), false);
    a.checkEqual("07. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), false);
}

/** Test mine laying with an empty ship.
    This verifies that we can deal with unknown data.
    A: call checkLayMission with a default-initialized ship.
    E: must report false. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:empty-ship", a)
{
    MinefieldMission testee;
    Ship ship(77);
    game::map::Universe univ;
    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)));
    game::UnitScoreDefinitionList shipScores;     // required for hull functions, which are required to determine FCode availability
    game::spec::ShipList shipList;                // required for fcodes and hull functions

    a.checkEqual("01. checkLayMission", testee.checkLayMission(ship, univ, *root, game::map::Configuration(), shipScores, shipList), false);
}

/** Test mine laying with a freighter.
    A: call checkLayMission with a freighter.
    E: must report false. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:freighter", a)
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addFreighter(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t(""));
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), false);
}

/** Test mine laying with wrong mission.
    A: call checkLayMission with a torper that has mission Explore.
    E: must report false. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:other-mission", a)
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(1, 0, 0);
    sh.setFriendlyCode(String_t(""));
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), false);
}

/** Test mine laying (successful base case).
    A: call checkLayMission with a torper that has mission Lay Mines.
    E: must report new minefield being laid. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:normal", a)
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t(""));
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0);
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 3);
    a.checkEqual("13. isWeb", testee.isWeb(), false);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 60);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 60*49);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), false);
}

/** Test mine laying disabled in config.
    A: set AllowMinefields=No. Call checkLayMission with a torper that has mission Lay Mines.
    E: must report false. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:minelaying-disabled", a)
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t(""));
    h.turn.config()[HostConfiguration::AllowMinefields].set(false);
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), false);
}

/** Test mine laying as robots.
    This verifies that UnitsPerTorpRate is correctly handled.
    A: call checkLayMission with a Robotic torper that has mission Lay Mines.
    E: must report new minefield being laid with large rate. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:robot", a)
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 9);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t(""));
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0);
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 9);
    a.checkEqual("13. isWeb", testee.isWeb(), false);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 60);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 4*60*49); /* 4x bonus applied */
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), false);
}

/** Test mine laying using "mdX" friendly codes.
    A: call checkLayMission with a torper that has mission Lay Mines and an "mdX" friendly code.
    E: must report new minefield being laid with correct number of torps. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:md-fcode", a)
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
        a(c.friendlyCode).checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

        a(c.friendlyCode).checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0);
        a(c.friendlyCode).checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 3);
        a(c.friendlyCode).checkEqual("13. isWeb", testee.isWeb(), false);
        a(c.friendlyCode).checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), c.expected);
        a(c.friendlyCode).checkEqual("15. getNumUnits", testee.getNumUnits(), c.expected*49);
        a(c.friendlyCode).checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
        a(c.friendlyCode).checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), true);
    }
}

/** Test mine laying using disallowed "mdX" friendly code.
    A: Set friendly-code "mdh" to registered-only, but don't add a key. Call checkLayMission on ship with "mdh" fcode.
    E: must report minefield laid with all torps (fcode ignored). */
AFL_TEST("game.map.MinefieldMission:checkLayMission:md-fcode:disabled", a)
{
    MinefieldMission testee;
    TestHarness h;
    afl::string::NullTranslator tx;

    h.turn.shipList().friendlyCodes().addCode(game::spec::FriendlyCode("mdh", "rs,drop half", tx));

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t("mdh"));
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0);
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 3);
    a.checkEqual("13. isWeb", testee.isWeb(), false);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 60);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 60*49);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), false);
}

/** Test mine laying using inapplicable "mdX" friendly code.
    A: Set friendly-code "mdh" to planets-only. Call checkLayMission on ship with "mdh" fcode.
    E: must report minefield laid with all torps (fcode ignored). */
AFL_TEST("game.map.MinefieldMission:checkLayMission:md-fcode:not-applicable", a)
{
    MinefieldMission testee;
    TestHarness h;
    afl::string::NullTranslator tx;

    h.turn.shipList().friendlyCodes().addCode(game::spec::FriendlyCode("mdh", "p,drop half", tx));

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t("mdh"));
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0);
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 3);
    a.checkEqual("13. isWeb", testee.isWeb(), false);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 60);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 60*49);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), false);
}

/** Test mine laying using "miX" friendly code.
    A: Call checkLayMission on ship with "miX" fcode.
    E: must report minefield laid with changed owner. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:mi-fcode", a)
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t("mi4"));
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0);
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 4);
    a.checkEqual("13. isWeb", testee.isWeb(), false);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 60);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 60*49);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), true);
}

/** Test mine laying using "miX" friendly code, Robot case.
    A: Call checkLayMission on Robotic ship with "miX" fcode.
    E: must report minefield laid with changed owner, 4x bonus not applied. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:mi-fcode:robot", a)
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 9);
    sh.setMission(3, 0, 0);
    sh.setFriendlyCode(String_t("mi4"));
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0);
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 4);
    a.checkEqual("13. isWeb", testee.isWeb(), false);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 60);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 60*49);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), true);
}

/** Test laying web mines.
    A: Call checkLayMission on Tholian ship with mission 9.
    E: must report web field laid. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:web", a)
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 7);
    sh.setMission(9, 0, 0);
    sh.setFriendlyCode(String_t(""));
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0);
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 7);
    a.checkEqual("13. isWeb", testee.isWeb(), true);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 60);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 60*49);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), false);
}

/** Test laying web mines disabled in config.
    A: Set AllowWebMines=No. Call checkLayMission on Tholian ship with mission 9.
    E: must report false. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:web:disabled", a)
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 9);
    sh.setMission(9, 0, 0);
    sh.setFriendlyCode(String_t(""));
    h.turn.config()[HostConfiguration::AllowWebMines].set(false);
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), false);
}

/** Test laying web mines, other race.
    A: Call checkLayMission on non-Tholian ship with mission 9.
    E: must report false. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:web:wrong-race", a)
{
    MinefieldMission testee;
    TestHarness h;

    Ship& sh = addTorper(h, 222, 4);
    sh.setMission(9, 0, 0);
    sh.setFriendlyCode(String_t(""));
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), false);
}

/** Test laying minefield using "Lay Mines" extended mission.
    A: Call checkLayMission on ship with mission "Lay Mines" and parameters.
    E: must report new minefield being laid with given parameters. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:lay-extended", a)
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(21, 12, 4);                    /* pmsn_LayMines + default ExtMissionsStartAt */
    sh.setFriendlyCode(String_t("mi5"));         /* not relevant here */
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0);
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 4); /* from mission */
    a.checkEqual("13. isWeb", testee.isWeb(), false);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 12);  /* from mission */
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 12*49);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), false);
}

/** Test laying minefield using "Lay Web Mines" extended mission.
    A: Call checkLayMission on ship with mission "Lay Web Mines" and parameters.
    E: must report new web field being laid with given parameters. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:lay-web-extended", a)
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    Ship& sh = addTorper(h, 222, 7);
    sh.setMission(22, 12, 9);                    /* pmsn_LayWeb + default ExtMissionsStartAt */
    sh.setFriendlyCode(String_t("mi5"));         /* not relevant here */
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0);
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 9); /* from mission */
    a.checkEqual("13. isWeb", testee.isWeb(), true);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 12);  /* from mission */
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 12*49);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), false);
}

/** Test laying minefield using "Add Mines To" extended mission.
    A: Call checkLayMission on ship with mission "Add Mines To" and parameters.
    E: must report new minefield being laid with given parameters. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:add-mines-to", a)
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(36, 17, 0);                    /* pmsn_LayMinesIn + default ExtMissionsStartAt */
    sh.setFriendlyCode(String_t("mi5"));
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0);
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 5);
    a.checkEqual("13. isWeb", testee.isWeb(), false);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 17);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 17*49);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), true);
}

/** Test laying minefield using "Add Web Mines To" extended mission.
    A: Call checkLayMission on ship with mission "Add Web Mines To" and parameters.
    E: must report new web field being laid with given parameters. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:add-web-mines-to", a)
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    Ship& sh = addTorper(h, 222, 7);
    sh.setMission(37, 17, 0);                    /* pmsn_LayWWebIn + default ExtMissionsStartAt */
    sh.setFriendlyCode(String_t("md3"));         /* not relevant */
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0);
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 7);
    a.checkEqual("13. isWeb", testee.isWeb(), true);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 17);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 17*49);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), false);
}

/** Test extending a minefield, Host case.
    A: Create minefields. Use Tim-Host. Call checkLayMission on ship with mission "Lay Mines".
    E: must report closest minefield being extended. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:extend-minefield:host", a)
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

    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 30); // closest
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 3);
    a.checkEqual("13. isWeb", testee.isWeb(), false);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 60);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 60*49);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), false);
}

/** Test extending a minefield, Host failure case.
    A: Create minefields, closest does not overlap ship. Use Tim-Host. Call checkLayMission on ship with mission "Lay Mines".
    E: must report new minefield being laid. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:extend-minefield:host:fail", a)
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

    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0); // make new field
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 3);
    a.checkEqual("13. isWeb", testee.isWeb(), false);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 60);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 60*49);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), false);
}

/** Test extending a minefield, PHost case.
    A: Create minefields. Use PHost. Call checkLayMission on ship with mission "Lay Mines".
    E: must report lowest-Id minefield being extended. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:extend-minefield:phost", a)
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

    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 10); // first matching
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 3);
    a.checkEqual("13. isWeb", testee.isWeb(), false);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 60);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 60*49);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), false);
}

/** Test extending a minefield, PHost with extended mission.
    A: Create minefields. Use PHost. Call checkLayMission on ship with mission "Add Mines To" and explicitly given Id.
    E: must report selected Id being extended. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:extend-minefield:add-mines-to", a)
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

    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 20); // selected
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 3);
    a.checkEqual("13. isWeb", testee.isWeb(), false);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 60);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 60*49);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), false);
}

/** Test extending a minefield, PHost with extended mission, failure case.
    A: Call checkLayMission on ship with mission "Add Mines To" and given Id of non-existing field.
    E: must report false. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:extend-minefield:add-mines-to:wrong-id", a)
{
    MinefieldMission testee;
    TestHarness h(RegistrationKey::Registered);

    Ship& sh = addTorper(h, 222, 3);
    sh.setMission(36, 17, 444);                   /* pmsn_LayMinesIn + default ExtMissionsStartAt */
    sh.setFriendlyCode(String_t(""));
    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), false);
}

/** Test extending a minefield, PHost with extended mission, failure case.
    A: Create minefield not overlapping the ship. Call checkLayMission on ship with mission "Add Mines To" and given Id of that field.
    E: must report false. */
AFL_TEST("game.map.MinefieldMission:checkLayMission:extend-minefield:add-mines-to:wrong-position", a)
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

    a.checkEqual("01. checkLayMission", testee.checkLayMission(sh, h.turn.universe(), h.turn.version(), h.key, h.turn.mapConfiguration(), h.turn.config(), h.shipScores, h.turn.shipList()), false);
}

/** Test mine scooping with an empty ship.
    This verifies that we can deal with unknown data.
    A: call checkScoopMission with a default-initialized ship.
    E: must report false. */
AFL_TEST("game.map.MinefieldMission:checkScoopMission:empty", a)
{
    MinefieldMission testee;
    Ship ship(77);
    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)));
    game::UnitScoreDefinitionList shipScores;     // required for hull functions, which are required to determine FCode availability
    game::spec::ShipList shipList;                // required for fcodes and hull functions

    a.checkEqual("01. checkScoopMission", testee.checkScoopMission(ship, *root, shipScores, shipList), false);
}

/** Test mine scooping with a freighter.
    A: call checkScoopMission with a freighter.
    E: must report false. */
AFL_TEST("game.map.MinefieldMission:checkScoopMission:freighter", a)
{
    MinefieldMission testee;

    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)), RegistrationKey::Registered);
    TestHarness h;
    Ship& ship = addFreighter(h, 222, 3);
    ship.setMission(2, 0, 0);
    ship.setFriendlyCode(String_t("msc"));

    a.checkEqual("01. checkScoopMission", testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), false);
}

/** Test mine scooping (successful base case).
    A: call checkScoopMission on a ship with torps, beams, and "msc".
    E: must report success. */
AFL_TEST("game.map.MinefieldMission:checkScoopMission:msc", a)
{
    MinefieldMission testee;

    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)));
    TestHarness h;
    Ship& ship = addTorper(h, 222, 3);
    ship.setNumBeams(2);
    ship.setBeamType(5);
    ship.setMission(2, 0, 0);
    ship.setFriendlyCode(String_t("msc"));

    a.checkEqual("01. checkScoopMission", testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 0);
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 3);
    a.checkEqual("13. isWeb", testee.isWeb(), false);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 0);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 0);
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), true);
}

/** Test mine scooping, inapplicable friendly code.
    A: Define "msc" as planet-only fcode. Call checkScoopMission on a ship with torps, beams, and "msc".
    E: must report false. */
AFL_TEST("game.map.MinefieldMission:checkScoopMission:msc:inapplicable", a)
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

    a.checkEqual("01. checkScoopMission", testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), false);
}

/** Test mine scooping, unusable friendly code.
    A: Define "msc" as registered-only fcode. Add unregistered key. Call checkScoopMission on a ship with torps, beams, and "msc".
    E: must report false. */
AFL_TEST("game.map.MinefieldMission:checkScoopMission:msc:unregistered", a)
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

    a.checkEqual("01. checkScoopMission", testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), false);
}

/** Test mine scooping, no beams.
    A: Use Host. Call checkScoopMission on a ship with torps, no beams, and "msc".
    E: must report success. */
AFL_TEST("game.map.MinefieldMission:checkScoopMission:msc:no-beams:host", a)
{
    MinefieldMission testee;

    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::Host, MKVERSION(3,22,40)));
    TestHarness h;
    Ship& ship = addTorper(h, 222, 3);
    ship.setNumBeams(0);
    ship.setBeamType(0);
    ship.setMission(2, 0, 0);
    ship.setFriendlyCode(String_t("msc"));

    a.checkEqual("01. checkScoopMission", testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), true);
}

/** Test mine scooping, no-beams.
    A: Use PHost. Call checkScoopMission on a ship with torps, no beams, and "msc".
    E: must report false. */
AFL_TEST("game.map.MinefieldMission:checkScoopMission:msc:no-beams:phost", a)
{
    MinefieldMission testee;

    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,22,40)));
    TestHarness h;
    Ship& ship = addTorper(h, 222, 3);
    ship.setNumBeams(0);
    ship.setBeamType(0);
    ship.setMission(2, 0, 0);
    ship.setFriendlyCode(String_t("msc"));

    a.checkEqual("01. checkScoopMission", testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), false);
}

/** Test mine scooping using mission.
    A: Use PHost and registered key. Call checkScoopMission on a ship with "Scoop Torpedoes" mission.
    E: must report success. */
AFL_TEST("game.map.MinefieldMission:checkScoopMission:mission", a)
{
    MinefieldMission testee;

    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)), RegistrationKey::Registered);
    TestHarness h;
    Ship& ship = addTorper(h, 222, 3);
    ship.setNumBeams(2);
    ship.setBeamType(5);
    ship.setMission(23, 27, 456);                 /* ExtMissionsStartAt + pmsn_ScoopTorps */
    ship.setFriendlyCode(String_t(""));

    a.checkEqual("01. checkScoopMission", testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), true);

    a.checkEqual("11. getRequiredMinefieldId", testee.getRequiredMinefieldId(), 456);
    a.checkEqual("12. getMinefieldOwner", testee.getMinefieldOwner(), 3);
    a.checkEqual("13. isWeb", testee.isWeb(), false);
    a.checkEqual("14. getNumTorpedoes", testee.getNumTorpedoes(), 27);
    a.checkEqual("15. getNumUnits", testee.getNumUnits(), 0);    /* not relevant for scooping */
    a.checkEqual("16. isMissionUsed", testee.isMissionUsed(), true);
    a.checkEqual("17. isFriendlyCodeUsed", testee.isFriendlyCodeUsed(), false);
}

/** Test mine scooping using mission, unregistered.
    A: Use PHost and unregistered key. Call checkScoopMission on a ship with "Scoop Torpedoes" mission.
    E: must report false. */
AFL_TEST("game.map.MinefieldMission:checkScoopMission:mission:unregistered", a)
{
    MinefieldMission testee;

    afl::base::Ref<game::Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)), RegistrationKey::Unregistered);
    TestHarness h;
    Ship& ship = addTorper(h, 222, 3);
    ship.setNumBeams(2);
    ship.setBeamType(5);
    ship.setMission(23, 27, 456);                 /* ExtMissionsStartAt + pmsn_ScoopTorps */
    ship.setFriendlyCode(String_t(""));

    a.checkEqual("01. checkScoopMission", testee.checkScoopMission(ship, *root, h.shipScores, h.turn.shipList()), false);
}
