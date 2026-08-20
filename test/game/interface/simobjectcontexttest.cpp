/**
  *  \file test/game/interface/simobjectcontexttest.cpp
  *  \brief Test for game::interface::SimObjectContext
  */

#include "game/interface/simobjectcontext.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/sim/session.hpp"
#include "game/test/defaultshiplist.hpp"
#include "game/test/root.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/values.hpp"

using afl::base::Ref;
using afl::data::Segment;
using game::Root;
using game::sim::Object;
using game::sim::Planet;
using game::sim::Session;
using game::sim::Ship;
using game::spec::ShipList;
using interpreter::Arguments;
using interpreter::Context;
using interpreter::Error;
using interpreter::test::ContextVerifier;
using interpreter::test::ValueVerifier;
using interpreter::test::verifyNewBoolean;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;

namespace {
    void initDefaults(Root& root, ShipList& shipList)
    {
        for (int i = 1; i <= 11; ++i) {
            root.playerList().create(i);
        }
        game::test::initDefaultShipList(shipList);
    }
}

/** Test behavior with torpedo ship. */
AFL_TEST("game.interface.SimObjectContext:torpedo-ship", a)
{
    Ref<Session> session = *new Session();
    Ref<Root> root = game::test::makeRoot(game::HostVersion());
    Ref<ShipList> shipList = *new ShipList();
    afl::string::NullTranslator tx;

    initDefaults(*root, *shipList);

    Ship& sh = session->setup().addShip();
    sh.setId(69);
    sh.setName("USS Cerritos");
    sh.setFriendlyCode("xyz");
    sh.setDamage(21);
    sh.setShield(77);
    sh.setOwner(2);
    sh.setExperienceLevel(1);
    sh.setFlags(Object::fl_Cloaked | Object::fl_Commander | Object::fl_CommanderSet);
    sh.setFlakRatingOverride(999);
    sh.setFlakCompensationOverride(666);

    sh.setHullType(15, *shipList);
    sh.setCrew(30);
    sh.setBeamType(3);
    sh.setNumBeams(4);
    sh.setTorpedoType(5);
    sh.setNumLaunchers(6);
    sh.setAmmo(120);
    sh.setEngineType(9);
    sh.setAggressiveness(11);
    sh.setInterceptId(42);

    game::interface::SimObjectContext testee(session, root, shipList, 0, tx);

    ContextVerifier verif(testee, a("SimObjectContext"));
    verif.verifyBasics();
    verif.verifyNotSerializable();
    verif.verifyTypes();

    // Verify values
    verif.verifyInteger("AGGRESSIVENESS",    11);
    verif.verifyString ("AUX",               "Mark 3 Photon");
    verif.verifyInteger("AUX$",              5);
    verif.verifyInteger("AUX.AMMO",          120);
    verif.verifyInteger("AUX.COUNT",         6);
    verif.verifyBoolean("BASE.YESNO",        false);
    verif.verifyString ("BEAM",              "Plasma Bolt");
    verif.verifyInteger("BEAM$",             3);
    verif.verifyInteger("BEAM.COUNT",        4);
    verif.verifyBoolean("CLOAKED",           true);
    verif.verifyInteger("CREW",              30);
    verif.verifyInteger("DAMAGE",            21);
    verif.verifyBoolean("DEACTIVATED",       false);
    verif.verifyNull   ("DEFENSE");
    verif.verifyNull   ("DEFENSE.BASE");
    verif.verifyString ("ENGINE",            "Transwarp Drive");
    verif.verifyInteger("ENGINE$",           9);
    verif.verifyString ("FCODE",             "xyz");
    verif.verifyInteger("FIGHTER.BAYS",      0);
    verif.verifyInteger("FIGHTER.COUNT",     0);
    verif.verifyInteger("FLAGS",             6176); // Flag values are part of external interface
    verif.verifyString ("HULL",              "SMALL DEEP SPACE FREIGHTER");
    verif.verifyInteger("HULL$",             15);
    verif.verifyInteger("ID",                69);
    verif.verifyInteger("LEVEL",             1);
    verif.verifyInteger("MASS",              30);
    verif.verifyInteger("MISSION.INTERCEPT", 42);
    verif.verifyString ("NAME",              "USS Cerritos");
    verif.verifyString ("OWNER",             "Player 2");
    verif.verifyInteger("OWNER$",            2);
    verif.verifyInteger("RATING.C",          666);
    verif.verifyInteger("RATING.R",          999);
    verif.verifyInteger("SHIELD",            77);
    verif.verifyNull   ("STORAGE.AMMO");
    verif.verifyNull   ("TECH.BEAM");
    verif.verifyNull   ("TECH.TORPEDO");
    verif.verifyString ("TORP",              "Mark 3 Photon");
    verif.verifyInteger("TORP$",             5);
    verif.verifyInteger("TORP.COUNT",        120);
    verif.verifyInteger("TORP.LCOUNT",       6);
    verif.verifyString ("TYPE",              "Ship");

    // We cannot assign
    AFL_CHECK_THROWS(a("01. assign BEAM"), verif.setStringValue("BEAM", "foo"), Error);
    AFL_CHECK_THROWS(a("02. assign TYPE"), verif.setIntegerValue("BEAM", 9), Error);

    // Failure to look up
    Context::PropertyIndex_t idx;
    a.checkNull("11. lookup failure", testee.lookup("NARF", idx));

    // Obtain 'HasFunction' and verify it
    std::auto_ptr<afl::data::Value> hasFunction(verif.getValue("HASFUNCTION"));
    interpreter::IndexableValue* indexableFunction = dynamic_cast<interpreter::IndexableValue*>(hasFunction.get());
    a.checkNonNull("21. hasFunction", hasFunction.get());
    a.checkNonNull("22. indexableFunction", indexableFunction);

    ValueVerifier(*indexableFunction, a("23. hasFunction")).verifyBasics();
    ValueVerifier(*indexableFunction, a("24. hasFunction")).verifyNotSerializable();

    // Easy attributes
    a.checkEqual("31. hasFunction dim", indexableFunction->getDimension(0), 0U);
    AFL_CHECK_THROWS(a("32. hasFunction makeFirstContext"), indexableFunction->makeFirstContext(), Error);

    // Verify HasFunction("Commander")
    {
        Segment seg;
        seg.pushBackString("Commander");
        Arguments args(seg, 0, 1);
        verifyNewBoolean(a("41. get"), indexableFunction->get(args), true);
    }

    // Verify HasFunction("Commander", 1)
    {
        Segment seg;
        seg.pushBackString("Commander");
        seg.pushBackInteger(1);
        Arguments args(seg, 0, 2);
        verifyNewBoolean(a("42. get"), indexableFunction->get(args), false);
    }

    // Verify HasFunction("Elusive")
    {
        Segment seg;
        seg.pushBackString("Elusive");
        Arguments args(seg, 0, 1);
        verifyNewBoolean(a("43. get"), indexableFunction->get(args), false);
    }

    // Verify HasFunction(EMPTY)
    {
        Segment seg;
        Arguments args(seg, 0, 1);
        verifyNewNull(a("44. get"), indexableFunction->get(args));
    }

    // Verify HasFunction("BadAbility")
    {
        Segment seg;
        seg.pushBackString("BadAbility");
        Arguments args(seg, 0, 1);
        verifyNewNull(a("45. get"), indexableFunction->get(args));
    }

    // Verify bad type on arg 2
    {
        Segment seg;
        seg.pushBackString("Commander");
        seg.pushBackString("?");
        Arguments args(seg, 0, 2);
        AFL_CHECK_THROWS(a("46. get"), indexableFunction->get(args), Error);
    }

    // Verify range error on arg 2
    {
        Segment seg;
        seg.pushBackString("Commander");
        seg.pushBackInteger(9);
        Arguments args(seg, 0, 2);
        AFL_CHECK_THROWS(a("37. get"), indexableFunction->get(args), Error);
    }

    // Verify failure to assign
    {
        Segment seg;
        seg.pushBackString("Commander");
        Arguments args(seg, 0, 1);

        afl::data::IntegerValue iv(1);
        AFL_CHECK_THROWS(a("41. set"), indexableFunction->set(args, &iv), Error);
    }
}

/** Test behavior with carrier. */
AFL_TEST("game.interface.SimObjectContext:carrier", a)
{
    Ref<Session> session = *new Session();
    Ref<Root> root = game::test::makeRoot(game::HostVersion());
    Ref<ShipList> shipList = *new ShipList();
    afl::string::NullTranslator tx;

    initDefaults(*root, *shipList);

    Ship& sh = session->setup().addShip();
    sh.setId(69);
    sh.setName("USS Protostar");
    sh.setFriendlyCode("abc");
    sh.setDamage(21);
    sh.setShield(77);
    sh.setOwner(2);
    sh.setExperienceLevel(1);
    sh.setFlags(Object::fl_Deactivated);
    sh.setFlakRatingOverride(777);
    sh.setFlakCompensationOverride(666);

    sh.setHullType(15, *shipList);
    sh.setCrew(30);
    sh.setBeamType(3);
    sh.setNumBeams(4);
    sh.setTorpedoType(0);
    sh.setNumBays(12);
    sh.setAmmo(88);
    sh.setEngineType(9);
    sh.setAggressiveness(11);
    sh.setInterceptId(42);

    game::interface::SimObjectContext testee(session, root, shipList, 0, tx);

    ContextVerifier verif(testee, a("SimObjectContext"));
    verif.verifyBasics();
    verif.verifyNotSerializable();
    verif.verifyTypes();

    // Verify values
    verif.verifyInteger("AGGRESSIVENESS",    11);
    verif.verifyString ("AUX",               "Fighters");
    verif.verifyInteger("AUX$",              11);
    verif.verifyInteger("AUX.AMMO",          88);
    verif.verifyInteger("AUX.COUNT",         12);
    verif.verifyBoolean("BASE.YESNO",        false);
    verif.verifyString ("BEAM",              "Plasma Bolt");
    verif.verifyInteger("BEAM$",             3);
    verif.verifyInteger("BEAM.COUNT",        4);
    verif.verifyBoolean("CLOAKED",           false);
    verif.verifyInteger("CREW",              30);
    verif.verifyInteger("DAMAGE",            21);
    verif.verifyBoolean("DEACTIVATED",       true);
    verif.verifyNull   ("DEFENSE");
    verif.verifyNull   ("DEFENSE.BASE");
    verif.verifyString ("ENGINE",            "Transwarp Drive");
    verif.verifyInteger("ENGINE$",           9);
    verif.verifyString ("FCODE",             "abc");
    verif.verifyInteger("FIGHTER.BAYS",      12);
    verif.verifyInteger("FIGHTER.COUNT",     88);
    verif.verifyInteger("FLAGS",             64); // Flag values are part of external interface
    verif.verifyString ("HULL",              "SMALL DEEP SPACE FREIGHTER");
    verif.verifyInteger("HULL$",             15);
    verif.verifyInteger("ID",                69);
    verif.verifyInteger("LEVEL",             1);
    verif.verifyInteger("MASS",              30);
    verif.verifyInteger("MISSION.INTERCEPT", 42);
    verif.verifyString ("NAME",              "USS Protostar");
    verif.verifyString ("OWNER",             "Player 2");
    verif.verifyInteger("OWNER$",            2);
    verif.verifyInteger("RATING.C",          666);
    verif.verifyInteger("RATING.R",          777);
    verif.verifyInteger("SHIELD",            77);
    verif.verifyNull   ("STORAGE.AMMO");
    verif.verifyNull   ("TECH.BEAM");
    verif.verifyNull   ("TECH.TORPEDO");
    verif.verifyNull   ("TORP");
    verif.verifyInteger("TORP$",             0);
    verif.verifyInteger("TORP.COUNT",        0);
    verif.verifyInteger("TORP.LCOUNT",       0);
    verif.verifyString ("TYPE",              "Ship");

    // Obtain 'hasFunction' and verify it
    std::auto_ptr<afl::data::Value> hasFunction(verif.getValue("HASFUNCTION"));
    interpreter::IndexableValue* indexableFunction = dynamic_cast<interpreter::IndexableValue*>(hasFunction.get());
    a.checkNonNull("21. hasFunction", hasFunction.get());
    a.checkNonNull("22. indexableFunction", indexableFunction);

    // Verify HasFunction("Commander")
    {
        Segment seg;
        seg.pushBackString("Commander");
        Arguments args(seg, 0, 1);
        verifyNewBoolean(a("31. get"), indexableFunction->get(args), false);
    }
}

/** Test behavior with torpedo ship and missing context.
    This will run into a multitude of "else return null" branches. */
AFL_TEST("game.interface.SimObjectContext:missing-context", a)
{
    Ref<Session> session = *new Session();
    Ref<Root> root = game::test::makeRoot(game::HostVersion());
    Ref<ShipList> shipList = *new ShipList();
    afl::string::NullTranslator tx;

    // Note: in initDefaults here

    Ship& sh = session->setup().addShip();
    sh.setId(69);
    sh.setName("USS Null");
    sh.setFriendlyCode("xyz");
    sh.setDamage(21);
    sh.setShield(77);
    sh.setOwner(2);
    sh.setExperienceLevel(1);
    sh.setFlags(Object::fl_Cloaked | Object::fl_Commander | Object::fl_CommanderSet);
    sh.setFlakRatingOverride(999);
    sh.setFlakCompensationOverride(666);

    sh.setHullType(15, *shipList);
    sh.setCrew(30);
    sh.setBeamType(3);
    sh.setNumBeams(4);
    sh.setTorpedoType(5);
    sh.setNumLaunchers(6);
    sh.setAmmo(120);
    sh.setEngineType(9);
    sh.setAggressiveness(11);
    sh.setInterceptId(42);

    game::interface::SimObjectContext testee(session, root, shipList, 0, tx);

    ContextVerifier verif(testee, a("SimObjectContext"));
    verif.verifyBasics();
    verif.verifyNotSerializable();
    verif.verifyTypes();

    // Verify values
    verif.verifyInteger("AGGRESSIVENESS",    11);
    verif.verifyNull   ("AUX");
    verif.verifyInteger("AUX$",              5);
    verif.verifyInteger("AUX.AMMO",          120);
    verif.verifyInteger("AUX.COUNT",         6);
    verif.verifyBoolean("BASE.YESNO",        false);
    verif.verifyNull   ("BEAM");
    verif.verifyInteger("BEAM$",             3);
    verif.verifyInteger("BEAM.COUNT",        4);
    verif.verifyBoolean("CLOAKED",           true);
    verif.verifyInteger("CREW",              30);
    verif.verifyInteger("DAMAGE",            21);
    verif.verifyBoolean("DEACTIVATED",       false);
    verif.verifyNull   ("DEFENSE");
    verif.verifyNull   ("DEFENSE.BASE");
    verif.verifyNull   ("ENGINE");
    verif.verifyInteger("ENGINE$",           9);
    verif.verifyString ("FCODE",             "xyz");
    verif.verifyInteger("FIGHTER.BAYS",      0);
    verif.verifyInteger("FIGHTER.COUNT",     0);
    verif.verifyInteger("FLAGS",             6176); // Flag values are part of external interface
    verif.verifyNull   ("HULL");
    verif.verifyInteger("HULL$",             15);
    verif.verifyInteger("ID",                69);
    verif.verifyInteger("LEVEL",             1);
    verif.verifyInteger("MASS",              100);
    verif.verifyInteger("MISSION.INTERCEPT", 42);
    verif.verifyString ("NAME",              "USS Null");
    verif.verifyNull   ("OWNER");
    verif.verifyInteger("OWNER$",            2);
    verif.verifyInteger("RATING.C",          666);
    verif.verifyInteger("RATING.R",          999);
    verif.verifyInteger("SHIELD",            77);
    verif.verifyNull   ("STORAGE.AMMO");
    verif.verifyNull   ("TECH.BEAM");
    verif.verifyNull   ("TECH.TORPEDO");
    verif.verifyNull   ("TORP");
    verif.verifyInteger("TORP$",             5);
    verif.verifyInteger("TORP.COUNT",        120);
    verif.verifyInteger("TORP.LCOUNT",       6);
    verif.verifyString ("TYPE",              "Ship");
}

/** Test behavior with planet. */
AFL_TEST("game.interface.SimObjectContext:planet", a)
{
    Ref<Session> session = *new Session();
    Ref<Root> root = game::test::makeRoot(game::HostVersion());
    Ref<ShipList> shipList = *new ShipList();
    afl::string::NullTranslator tx;

    initDefaults(*root, *shipList);

    Planet& pl = session->setup().addPlanet();
    pl.setId(363);
    pl.setName("Rambo 3");
    pl.setFriendlyCode("ggg");
    pl.setDamage(21);
    pl.setShield(77);
    pl.setOwner(7);
    pl.setExperienceLevel(4);
    pl.setFlags(Object::fl_TripleBeamKill | Object::fl_TripleBeamKillSet);
    pl.setFlakRatingOverride(444);
    pl.setFlakCompensationOverride(555);

    pl.setDefense(52);
    pl.setBaseDefense(99);
    pl.setBaseBeamTech(3);
    pl.setBaseTorpedoTech(7);
    pl.setNumBaseFighters(5);
    pl.setNumBaseTorpedoes(7, 50);

    game::interface::SimObjectContext testee(session, root, shipList, 0, tx);

    ContextVerifier verif(testee, a("SimObjectContext"));
    verif.verifyBasics();
    verif.verifyNotSerializable();
    verif.verifyTypes();

    // Verify values
    verif.verifyNull   ("AGGRESSIVENESS");
    verif.verifyNull   ("AUX");
    verif.verifyNull   ("AUX$");
    verif.verifyNull   ("AUX.AMMO");
    verif.verifyNull   ("AUX.COUNT");
    verif.verifyBoolean("BASE.YESNO",        true);
    verif.verifyNull   ("BEAM");
    verif.verifyNull   ("BEAM$");
    verif.verifyNull   ("BEAM.COUNT");
    verif.verifyNull   ("CLOAKED");
    verif.verifyNull   ("CREW");
    verif.verifyInteger("DAMAGE",            21);
    verif.verifyBoolean("DEACTIVATED",       false);
    verif.verifyInteger("DEFENSE",           52);
    verif.verifyInteger("DEFENSE.BASE",      99);
    verif.verifyNull   ("ENGINE");
    verif.verifyNull   ("ENGINE$");
    verif.verifyString ("FCODE",             "ggg");
    verif.verifyNull   ("FIGHTER.BAYS");
    verif.verifyInteger("FIGHTER.COUNT",     5);
    verif.verifyInteger("FLAGS",             3*65536); // Flag values are part of external interface
    verif.verifyNull   ("HULL");
    verif.verifyNull   ("HULL$");
    verif.verifyInteger("ID",                363);
    verif.verifyInteger("LEVEL",             4);
    verif.verifyNull   ("MASS");
    verif.verifyNull   ("MISSION.INTERCEPT");
    verif.verifyString ("NAME",              "Rambo 3");
    verif.verifyString ("OWNER",             "Player 7");
    verif.verifyInteger("OWNER$",            7);
    verif.verifyInteger("RATING.C",          555);
    verif.verifyInteger("RATING.R",          444);
    verif.verifyInteger("SHIELD",            77);
    verif.verifyInteger("TECH.BEAM",         3);
    verif.verifyInteger("TECH.TORPEDO",      7);
    verif.verifyNull   ("TORP");
    verif.verifyNull   ("TORP$");
    verif.verifyNull   ("TORP.COUNT");
    verif.verifyNull   ("TORP.LCOUNT");
    verif.verifyString ("TYPE",              "Planet");

    // We cannot assign
    AFL_CHECK_THROWS(a("01. assign BEAM"), verif.setStringValue("BEAM", "foo"), Error);
    AFL_CHECK_THROWS(a("02. assign TYPE"), verif.setIntegerValue("BEAM", 9), Error);

    // Failure to look up
    Context::PropertyIndex_t idx;
    a.checkNull("11. lookup failure", testee.lookup("NARF", idx));

    // Obtain 'HasFunction' and verify it
    std::auto_ptr<afl::data::Value> hasFunction(verif.getValue("HASFUNCTION"));
    interpreter::IndexableValue* indexableFunction = dynamic_cast<interpreter::IndexableValue*>(hasFunction.get());
    a.checkNonNull("21. hasFunction", hasFunction.get());
    a.checkNonNull("22. indexableFunction", indexableFunction);

    ValueVerifier(*indexableFunction, a("23. hasFunction")).verifyBasics();
    ValueVerifier(*indexableFunction, a("24. hasFunction")).verifyNotSerializable();

    // Easy attributes
    a.checkEqual("31. hasFunction dim", indexableFunction->getDimension(0), 0U);
    AFL_CHECK_THROWS(a("32. hasFunction makeFirstContext"), indexableFunction->makeFirstContext(), Error);

    // Verify HasFunction("TripleBeamKill")
    {
        Segment seg;
        seg.pushBackString("TripleBeamKill");
        Arguments args(seg, 0, 1);
        verifyNewBoolean(a("41. get"), indexableFunction->get(args), true);
    }

    // Same thing now for Storage.Ammo
    std::auto_ptr<afl::data::Value> storageAmmo(verif.getValue("STORAGE.AMMO"));
    interpreter::IndexableValue* indexableAmmo = dynamic_cast<interpreter::IndexableValue*>(storageAmmo.get());
    a.checkNonNull("51. storageAmmo", storageAmmo.get());
    a.checkNonNull("52. indexableAmmo", indexableAmmo);

    ValueVerifier(*indexableAmmo, a("53. storageAmmo")).verifyBasics();
    ValueVerifier(*indexableAmmo, a("54. storageAmmo")).verifyNotSerializable();

    // Easy attributes
    a.checkEqual("61. storageAmmo dim", indexableAmmo->getDimension(0), 1U);
    a.checkEqual("62. storageAmmo dim", indexableAmmo->getDimension(1), 12U);
    AFL_CHECK_THROWS(a("63. storageAmmo makeFirstContext"), indexableAmmo->makeFirstContext(), Error);

    // Verify Storage.Ammo(7)
    {
        Segment seg;
        seg.pushBackInteger(7);
        Arguments args(seg, 0, 1);
        verifyNewInteger(a("71. get"), indexableAmmo->get(args), 50);
    }

    // Verify Storage.Ammo(11)
    {
        Segment seg;
        seg.pushBackInteger(11);
        Arguments args(seg, 0, 1);
        verifyNewInteger(a("72. get"), indexableAmmo->get(args), 5);
    }

    // Verify Storage.Ammo arity error
    {
        Segment seg;
        Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("73. get"), indexableAmmo->get(args), Error);
    }

    // Verify Storage.Ammo type error
    {
        Segment seg;
        seg.pushBackString("foo");
        Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("74. get"), indexableAmmo->get(args), Error);
    }

    // Verify Storage.Ammo with null argument
    {
        Segment seg;
        Arguments args(seg, 0, 1);
        verifyNewNull(a("75. get"), indexableAmmo->get(args));
    }
}

/** Test behavior with out-of-range index.
    This cannot normally happen. */
AFL_TEST("game.interface.SimObjectContext:out-of-range", a)
{
    Ref<Session> session = *new Session();
    Ref<Root> root = game::test::makeRoot(game::HostVersion());
    Ref<ShipList> shipList = *new ShipList();
    afl::string::NullTranslator tx;

    // Note: in initDefaults here
    // Note: no setup().addShip() here

    game::interface::SimObjectContext testee(session, root, shipList, 0, tx);

    ContextVerifier verif(testee, a("SimObjectContext"));

    // Verify values
    verif.verifyNull   ("AGGRESSIVENESS");
    verif.verifyNull   ("AUX");
    verif.verifyNull   ("AUX$");
    verif.verifyNull   ("AUX.AMMO");
    verif.verifyNull   ("AUX.COUNT");
    verif.verifyBoolean("BASE.YESNO", 0);
    verif.verifyNull   ("BEAM");
    verif.verifyNull   ("BEAM$");
    verif.verifyNull   ("BEAM.COUNT");
    verif.verifyNull   ("CLOAKED");
    verif.verifyNull   ("CREW");
    verif.verifyNull   ("DAMAGE");
    verif.verifyNull   ("DEACTIVATED");
    verif.verifyNull   ("DEFENSE");
    verif.verifyNull   ("DEFENSE.BASE");
    verif.verifyNull   ("ENGINE");
    verif.verifyNull   ("ENGINE$");
    verif.verifyNull   ("FCODE");
    verif.verifyNull   ("FIGHTER.BAYS");
    verif.verifyNull   ("FIGHTER.COUNT");
    verif.verifyNull   ("FLAGS");
    verif.verifyNull   ("HULL");
    verif.verifyNull   ("HULL$");
    verif.verifyNull   ("ID");
    verif.verifyNull   ("LEVEL");
    verif.verifyNull   ("MASS");
    verif.verifyNull   ("MISSION.INTERCEPT");
    verif.verifyNull   ("NAME");
    verif.verifyNull   ("OWNER");
    verif.verifyNull   ("OWNER$");
    verif.verifyNull   ("RATING.C");
    verif.verifyNull   ("RATING.R");
    verif.verifyNull   ("SHIELD");
    verif.verifyNull   ("STORAGE.AMMO");
    verif.verifyNull   ("TECH.BEAM");
    verif.verifyNull   ("TECH.TORPEDO");
    verif.verifyNull   ("TORP");
    verif.verifyNull   ("TORP$");
    verif.verifyNull   ("TORP.COUNT");
    verif.verifyNull   ("TORP.LCOUNT");
    verif.verifyNull   ("TYPE");
}

/** Test iteration. */
AFL_TEST("game.interface.SimObjectContext:iteration", a)
{
    Ref<Session> session = *new Session();
    Ref<Root> root = game::test::makeRoot(game::HostVersion());
    Ref<ShipList> shipList = *new ShipList();
    afl::string::NullTranslator tx;

    initDefaults(*root, *shipList);
    session->setup().addShip().setName("first ship");
    session->setup().addShip().setName("second ship");
    session->setup().addPlanet().setName("planet");

    // Can access first item
    game::interface::SimObjectContext testee(session, root, shipList, 0, tx);
    ContextVerifier(testee, a("v1")).verifyString("NAME", "first ship");
    a.checkEqual("01. slot", testee.getSlotNumber(), 0U);

    // Can access second item
    a.check("11. next", testee.next());
    ContextVerifier(testee, a("v2")).verifyString("NAME", "second ship");
    a.checkEqual("12. slot", testee.getSlotNumber(), 1U);

    // Can access third item
    a.check("21. next", testee.next());
    ContextVerifier(testee, a("v3")).verifyString("NAME", "planet");
    a.checkEqual("22. slot", testee.getSlotNumber(), 2U);

    // Fin
    a.check("99. next", !testee.next());
}
