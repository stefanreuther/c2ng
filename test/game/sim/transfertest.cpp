/**
  *  \file test/game/sim/transfertest.cpp
  *  \brief Test for game::sim::Transfer
  */

#include "game/sim/transfer.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/hostversion.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/ship.hpp"
#include "game/test/simpleturn.hpp"
#include "game/unitscoredefinitionlist.hpp"

using afl::string::NullTranslator;
using game::Element;
using game::HostVersion;
using game::map::Object;
using game::map::Planet;
using game::map::Ship;
using game::sim::BaseTransfer;
using game::sim::Transfer;
using game::test::SimpleTurn;
using game::UnitScoreDefinitionList;

/** Test copy from an empty ship. */
AFL_TEST("game.sim.Transfer:copyShipFromGame:empty", a)
{
    // Environment
    UnitScoreDefinitionList shipScores;
    UnitScoreDefinitionList planetScores;
    NullTranslator tx;
    HostVersion h(HostVersion::Host, MKVERSION(3,22,0));
    SimpleTurn t;
    Ship* in = t.universe().ships().create(77);

    // Test
    Transfer tr(shipScores, planetScores, t.shipList(), t.config(), h, tx);
    game::sim::Ship out;
    a.checkEqual("01. copyShipFromGame", tr.copyShipFromGame(out, *in), false);
}

/** Test copy from a regular ship. */
AFL_TEST("game.sim.Transfer:copyShipFromGame:normal", a)
{
    // Environment
    UnitScoreDefinitionList shipScores;
    UnitScoreDefinitionList planetScores;
    NullTranslator tx;
    HostVersion h(HostVersion::Host, MKVERSION(3,22,0));
    SimpleTurn t;

    // Define a ship
    const int HULL_NR = 12;
    const int SHIP_ID = 111;
    const int PLAYER_NR = 9;
    t.setHull(HULL_NR);
    Ship& in = t.addShip(SHIP_ID, PLAYER_NR, Object::Playable);
    in.setFriendlyCode(String_t("abc"));
    in.setName(String_t("Carola"));
    in.setCrew(99);
    in.setNumBeams(3);
    in.setBeamType(4);
    in.setNumBays(7);
    in.setEngineType(9);
    in.setPrimaryEnemy(7);
    in.setMission(1, 0, 0);
    in.addShipSpecialFunction(t.shipList().modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Commander));

    // Transfer
    Transfer tr(shipScores, planetScores, t.shipList(), t.config(), h, tx);
    game::sim::Ship out;
    a.checkEqual("01. copyShipFromGame", tr.copyShipFromGame(out, in), true);

    // Verify
    a.checkEqual("11. getOwner",          out.getOwner(), PLAYER_NR);
    a.checkEqual("12. getHullType",       out.getHullType(), HULL_NR);
    a.checkEqual("13. getName",           out.getName(), "Carola");
    a.checkEqual("14. getFriendlyCode",   out.getFriendlyCode(), "abc");
    a.checkEqual("15. getCrew",           out.getCrew(), 99);
    a.checkEqual("16. getNumBeams",       out.getNumBeams(), 3);
    a.checkEqual("17. getBeamType",       out.getBeamType(), 4);
    a.checkEqual("18. getEngineType",     out.getEngineType(), 9);
    a.checkEqual("19. getNumLaunchers",   out.getNumLaunchers(), 0);
    a.checkEqual("20. getTorpedoType",    out.getTorpedoType(), 0);
    a.checkEqual("21. getNumBays",        out.getNumBays(), 7);
    a.checkEqual("22. getAmmo",           out.getAmmo(), 100);        // set to cargo room because it's not known
    a.checkEqual("23. getAggressiveness", out.getAggressiveness(), 7);
    a.checkEqual("24. getFlags",          out.getFlags(), game::sim::Ship::fl_CommanderSet | game::sim::Ship::fl_Commander);
}

/** Test copy to regular ship. */
AFL_TEST("game.sim.Transfer:copyShipToGame:normal", a)
{
    // Environment
    UnitScoreDefinitionList shipScores;
    UnitScoreDefinitionList planetScores;
    NullTranslator tx;
    HostVersion h(HostVersion::Host, MKVERSION(3,22,0));
    SimpleTurn t;

    // Define a ship
    const int HULL_NR = 12;
    const int SHIP_ID = 111;
    const int PLAYER_NR = 9;
    t.setHull(HULL_NR);
    Ship& out = t.addShip(SHIP_ID, PLAYER_NR, Object::Playable);

    // Define a simulator ship
    game::sim::Ship in;
    in.setOwner(PLAYER_NR);
    in.setHullTypeOnly(HULL_NR);
    in.setFriendlyCode(String_t("abc"));
    in.setName(String_t("Carola"));
    in.setCrew(99);
    in.setNumBeams(3);
    in.setBeamType(4);
    in.setNumBays(7);
    in.setEngineType(9);
    in.setAggressiveness(game::sim::Ship::agg_Kill);

    // Transfer
    Transfer tr(shipScores, planetScores, t.shipList(), t.config(), h, tx);
    a.checkEqual("01. copyShipToGame", tr.copyShipToGame(out, in, t.universe(), t.mapConfiguration()), true);

    // Verify changes
    a.checkEqual("11. getName",         out.getName(), "Carola");
    a.checkEqual("12. getFriendlyCode", out.getFriendlyCode().orElse(""), "abc");
    a.checkEqual("13. getMission",      out.getMission().orElse(-1), int(game::spec::Mission::msn_Kill));
}

/** Test copy to mismatching ship. */
AFL_TEST("game.sim.Transfer:copyShipToGame:mismatch", a)
{
    // Environment
    UnitScoreDefinitionList shipScores;
    UnitScoreDefinitionList planetScores;
    NullTranslator tx;
    HostVersion h(HostVersion::Host, MKVERSION(3,22,0));
    SimpleTurn t;

    // Define a ship
    const int HULL_NR = 12;
    const int SHIP_ID = 111;
    const int PLAYER_NR = 9;
    t.setHull(HULL_NR);
    Ship& out = t.addShip(SHIP_ID, PLAYER_NR, Object::Playable);

    // Define a mismatching simulator ship
    game::sim::Ship in;
    in.setOwner(PLAYER_NR+1);
    in.setHullTypeOnly(HULL_NR);

    // Test
    Transfer tr(shipScores, planetScores, t.shipList(), t.config(), h, tx);
    a.checkEqual("01. copyShipToGame", tr.copyShipToGame(out, in, t.universe(), t.mapConfiguration()), false);
}

/** Test copy to regular ship with fighter transfer. */
AFL_TEST("game.sim.Transfer:copyShipToGame:fighters", a)
{
    // Environment
    UnitScoreDefinitionList shipScores;
    UnitScoreDefinitionList planetScores;
    NullTranslator tx;
    HostVersion h(HostVersion::Host, MKVERSION(3,22,0));
    SimpleTurn t;

    // Define a ship
    const int HULL_NR = 12;
    const int SHIP_ID = 111;
    const int PLAYER_NR = 9;
    const int BASE_ID = 47;
    t.setHull(HULL_NR);
    Ship& out = t.addShip(SHIP_ID, PLAYER_NR, Object::Playable);
    out.setAmmo(50);
    out.setNumBays(7);
    t.shipList().hulls().get(HULL_NR)->setMaxCargo(2000);

    // Define a planet
    Planet& pl = t.addBase(BASE_ID, PLAYER_NR, Object::Playable);
    pl.setCargo(Element::Fighters, 30);

    // Define sim ship
    game::sim::Ship in;
    in.setOwner(PLAYER_NR);
    in.setHullTypeOnly(HULL_NR);
    in.setNumBays(7);
    in.setAmmo(60);             // 10 more than in universe

    // Transfer
    Transfer tr(shipScores, planetScores, t.shipList(), t.config(), h, tx);
    a.checkEqual("01. copyShipToGame", tr.copyShipToGame(out, in, t.universe(), t.mapConfiguration()), true);

    // Verify
    a.checkEqual("11. Fighters", pl.getCargo(Element::Fighters).orElse(0), 20);
    a.checkEqual("12. getAmmo", out.getAmmo().orElse(0), 60);
}

/** Test copy to regular ship with torpedo transfer. */
AFL_TEST("game.sim.Transfer:copyShipToGame:torpedoes", a)
{
    // Environment
    UnitScoreDefinitionList shipScores;
    UnitScoreDefinitionList planetScores;
    NullTranslator tx;
    HostVersion h(HostVersion::Host, MKVERSION(3,22,0));
    SimpleTurn t;

    // Define a ship
    const int HULL_NR = 12;
    const int SHIP_ID = 111;
    const int PLAYER_NR = 9;
    const int BASE_ID = 47;
    const int TORP_ID = 3;
    t.setHull(HULL_NR);
    Ship& out = t.addShip(SHIP_ID, PLAYER_NR, Object::Playable);
    out.setAmmo(50);
    out.setTorpedoType(TORP_ID);
    out.setNumLaunchers(5);
    t.shipList().hulls().get(HULL_NR)->setMaxCargo(2000);

    // Define a planet
    Planet& pl = t.addBase(BASE_ID, PLAYER_NR, Object::Playable);
    pl.setCargo(Element::fromTorpedoType(TORP_ID), 15);

    // Define sim ship
    game::sim::Ship in;
    in.setOwner(PLAYER_NR);
    in.setHullTypeOnly(HULL_NR);
    in.setTorpedoType(TORP_ID);
    in.setNumLaunchers(5);
    in.setAmmo(40);             // 10 less than in universe

    // Transfer
    Transfer tr(shipScores, planetScores, t.shipList(), t.config(), h, tx);
    a.checkEqual("01. copyShipToGame", tr.copyShipToGame(out, in, t.universe(), t.mapConfiguration()), true);

    // Verify
    a.checkEqual("11. Torpedoes", pl.getCargo(Element::fromTorpedoType(TORP_ID)).orElse(-1), 25);
    a.checkEqual("12. getAmmo", out.getAmmo().orElse(0), 40);
}

/** Test copy from an empty planet. */
AFL_TEST("game.sim.Transfer:copyPlanetFromGame:empty", a)
{
    // Environment
    UnitScoreDefinitionList shipScores;
    UnitScoreDefinitionList planetScores;
    NullTranslator tx;
    HostVersion h(HostVersion::Host, MKVERSION(3,22,0));
    SimpleTurn t;
    Planet* in = t.universe().planets().create(77);

    // Test
    Transfer tr(shipScores, planetScores, t.shipList(), t.config(), h, tx);
    game::sim::Planet out;
    a.checkEqual("01. copyPlanetFromGame", tr.copyPlanetFromGame(out, *in), false);
}

/** Test copy from a regular planet. */
AFL_TEST("game.sim.Transfer:copyPlanetFromGame:normal", a)
{
    // Environment
    UnitScoreDefinitionList shipScores;
    UnitScoreDefinitionList planetScores;
    NullTranslator tx;
    HostVersion h(HostVersion::Host, MKVERSION(3,22,0));
    SimpleTurn t;

    // Define a planet
    const int PLANET_ID = 111;
    const int PLAYER_NR = 9;
    Planet& in = t.addPlanet(PLANET_ID, PLAYER_NR, Object::Playable);
    in.setFriendlyCode(String_t("efg"));
    in.setName(String_t("Florida"));
    in.setCargo(Element::Colonists, 171);

    // Transfer
    Transfer tr(shipScores, planetScores, t.shipList(), t.config(), h, tx);
    game::sim::Planet out;
    a.checkEqual("01. copyPlanetFromGame", tr.copyPlanetFromGame(out, in), true);

    // Verify
    a.checkEqual("11. getOwner",           out.getOwner(), PLAYER_NR);
    a.checkEqual("12. getName",            out.getName(), "Florida");
    a.checkEqual("13. getFriendlyCode",    out.getFriendlyCode(), "efg");
    a.checkEqual("14. getDefense",         out.getDefense(), 61);            // from colonists
    a.checkEqual("15. getBaseBeamTech",    out.getBaseBeamTech(), 0);
    a.checkEqual("16. getBaseTorpedoTech", out.getBaseTorpedoTech(), 0);
    a.checkEqual("17. getNumBaseFighters", out.getNumBaseFighters(), 0);
    a.checkEqual("18. getFlags",           out.getFlags(), 0);
}

/** Test copy from a starbase. */
AFL_TEST("game.sim.Transfer:copyPlanetFromGame:base", a)
{
    // Environment
    UnitScoreDefinitionList shipScores;
    UnitScoreDefinitionList planetScores;
    NullTranslator tx;
    HostVersion h(HostVersion::Host, MKVERSION(3,22,0));
    SimpleTurn t;

    // Define a planet
    const int PLANET_ID = 111;
    const int PLAYER_NR = 9;
    Planet& in = t.addBase(PLANET_ID, PLAYER_NR, Object::Playable);
    in.setFriendlyCode(String_t("pqr"));
    in.setName(String_t("Cuba"));
    in.setCargo(Element::Colonists, 171);
    in.setCargo(Element::Fighters, 30);
    in.setNumBuildings(game::DefenseBuilding, 20);
    in.setNumBuildings(game::BaseDefenseBuilding, 120);
    in.setBaseTechLevel(game::BeamTech, 4);
    in.setBaseTechLevel(game::TorpedoTech, 7);

    // Transfer
    Transfer tr(shipScores, planetScores, t.shipList(), t.config(), h, tx);
    game::sim::Planet out;
    a.checkEqual("01. copyPlanetFromGame", tr.copyPlanetFromGame(out, in), true);

    // Verify
    a.checkEqual("11. getOwner",           out.getOwner(), PLAYER_NR);
    a.checkEqual("12. getName",            out.getName(), "Cuba");
    a.checkEqual("13. getFriendlyCode",    out.getFriendlyCode(), "pqr");
    a.checkEqual("14. getDefense",         out.getDefense(), 20);
    a.checkEqual("15. getBaseBeamTech",    out.getBaseBeamTech(), 4);
    a.checkEqual("16. getBaseTorpedoTech", out.getBaseTorpedoTech(), 7);
    a.checkEqual("17. getNumBaseFighters", out.getNumBaseFighters(), 30);
    a.checkEqual("18. getBaseDefense",     out.getBaseDefense(), 120);
    a.checkEqual("19. getFlags",           out.getFlags(), 0);
}

/** Test copy to a regular planet. */
AFL_TEST("game.sim.Transfer:copyPlanetToGame:normal", a)
{
    // Environment
    UnitScoreDefinitionList shipScores;
    UnitScoreDefinitionList planetScores;
    NullTranslator tx;
    HostVersion h(HostVersion::Host, MKVERSION(3,22,0));
    SimpleTurn t;

    // Define a planet
    const int PLANET_ID = 111;
    const int PLAYER_NR = 9;
    Planet& out = t.addPlanet(PLANET_ID, PLAYER_NR, Object::Playable);
    out.setFriendlyCode(String_t("efg"));
    out.setName(String_t("Florida"));

    // Transfer
    Transfer tr(shipScores, planetScores, t.shipList(), t.config(), h, tx);
    game::sim::Planet in;
    in.setId(PLANET_ID);
    in.setOwner(PLAYER_NR);
    in.setFriendlyCode("hij");
    a.checkEqual("01. copyPlanetToGame", tr.copyPlanetToGame(out, in), true);

    // Verify
    a.checkEqual("11. getFriendlyCode", out.getFriendlyCode().orElse(""), "hij");
}

/** Test copy to a mismatching planet. */
AFL_TEST("game.sim.Transfer:copyPlanetToGame:mismatch", a)
{
    // Environment
    UnitScoreDefinitionList shipScores;
    UnitScoreDefinitionList planetScores;
    NullTranslator tx;
    HostVersion h(HostVersion::Host, MKVERSION(3,22,0));
    SimpleTurn t;

    // Define a planet
    const int PLANET_ID = 111;
    const int PLAYER_NR = 9;
    Planet& out = t.addPlanet(PLANET_ID, PLAYER_NR, Object::Playable);
    out.setFriendlyCode(String_t("efg"));
    out.setName(String_t("Florida"));

    // Transfer
    Transfer tr(shipScores, planetScores, t.shipList(), t.config(), h, tx);
    game::sim::Planet in;
    in.setId(PLANET_ID);
    in.setOwner(PLAYER_NR+1);
    in.setFriendlyCode("hij");
    a.checkEqual("01. copyPlanetToGame", tr.copyPlanetToGame(out, in), false);

    // Verify
    a.checkEqual("11. getFriendlyCode", out.getFriendlyCode().orElse(""), "efg");
}

/** Test copy from battle ship. */
AFL_TEST("game.sim.Transfer:copyShipFromBattle", a)
{
    // Environment
    NullTranslator tx;
    SimpleTurn t;

    // Define a hull (inspired by TestGameVcrObject::testGuess)
    const int HULL_NR = 12;
    game::spec::Hull* p = t.shipList().hulls().create(HULL_NR);
    p->setMass(300);
    p->setMaxBeams(12);
    p->setNumBays(1);
    p->setExternalPictureNumber(3);
    p->setInternalPictureNumber(77);

    // Define VCR object
    const int SHIP_ID = 111;
    const int PLAYER_NR = 9;
    game::vcr::Object obj;
    obj.setId(SHIP_ID);
    obj.setOwner(PLAYER_NR);
    obj.setPicture(3);
    obj.setMass(200);
    obj.setNumBeams(12);
    obj.setNumBays(3);
    obj.setIsPlanet(false);
    obj.setName("Oneoneone");
    a.checkEqual("01. getGuessedHull", obj.getGuessedHull(t.shipList().hulls()), HULL_NR);

    // Perform the copy
    game::sim::Ship ship;
    BaseTransfer tr(t.shipList(), t.config(), tx);
    bool ok = tr.copyShipFromBattle(ship, obj, HULL_NR, false);
    a.check("11. copyShipFromBattle", ok);

    // Verify
    a.checkEqual("21. getHullType", ship.getHullType(), HULL_NR);
    a.checkEqual("22. getId",       ship.getId(), SHIP_ID);
    a.checkEqual("23. getOwner",    ship.getOwner(), PLAYER_NR);
    a.checkEqual("24. getNumBeams", ship.getNumBeams(), 12);
}

/** Test copy from battle planet. */
AFL_TEST("game.sim.Transfer:copyPlanetFromBattle", a)
{
    // Environment
    NullTranslator tx;
    SimpleTurn t;

    // Define VCR object (derived from TestGameVcrObjectInfo::testPlanet3)
    game::vcr::Object o;
    o.setMass(281);
    o.setShield(100);
    o.setDamage(0);
    o.setCrew(0);
    o.setId(446);
    o.setOwner(8);
    o.setBeamType(10);
    o.setNumBeams(8);
    o.setTorpedoType(0);
    o.setNumLaunchers(0);
    o.setNumBays(14);
    o.setNumFighters(29);
    o.setExperienceLevel(0);
    o.setIsPlanet(true);

    // Perform the copy
    game::sim::Planet planet;
    BaseTransfer tr(t.shipList(), t.config(), tx);
    bool ok = tr.copyPlanetFromBattle(planet, o);
    a.check("01. copyPlanetFromBattle", ok);

    // Verify
    a.checkEqual("11. getId",              planet.getId(), 446);
    a.checkEqual("12. getOwner",           planet.getOwner(), 8);
    a.checkEqual("13. getDefense",         planet.getDefense(), 90);
    a.checkEqual("14. getBaseDefense",     planet.getBaseDefense(), 91);
    a.checkEqual("15. getBaseBeamTech",    planet.getBaseBeamTech(), 10);
    a.checkEqual("16. getNumBaseFighters", planet.getNumBaseFighters(), 20);
}
