/**
  *  \file u/t_game_sim_transfer.cpp
  *  \brief Test for game::sim::Transfer
  */

#include "game/sim/transfer.hpp"

#include "t_game_sim.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/ship.hpp"
#include "game/test/simpleturn.hpp"
#include "game/unitscoredefinitionlist.hpp"
#include "game/hostversion.hpp"

using afl::string::NullTranslator;
using game::Element;
using game::HostVersion;
using game::map::Object;
using game::map::Planet;
using game::map::Ship;
using game::sim::Transfer;
using game::test::SimpleTurn;
using game::UnitScoreDefinitionList;

/** Test copy from an empty ship. */
void
TestGameSimTransfer::testCopyFromEmptyShip()
{
    // Environment
    UnitScoreDefinitionList scoreDefinitions;
    NullTranslator tx;
    HostVersion h(HostVersion::Host, MKVERSION(3,22,0));
    SimpleTurn t;
    Ship* in = t.universe().ships().create(77);

    // Test
    Transfer tr(scoreDefinitions, t.shipList(), t.config(), h, tx);
    game::sim::Ship out;
    TS_ASSERT_EQUALS(tr.copyShipFromGame(out, *in), false);
}

/** Test copy from a regular ship. */
void
TestGameSimTransfer::testCopyFromShip()
{
    // Environment
    UnitScoreDefinitionList scoreDefinitions;
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
    in.addShipSpecialFunction(t.shipList().modifiedHullFunctions().getFunctionIdFromHostId(game::spec::HullFunction::Commander));

    // Transfer
    Transfer tr(scoreDefinitions, t.shipList(), t.config(), h, tx);
    game::sim::Ship out;
    TS_ASSERT_EQUALS(tr.copyShipFromGame(out, in), true);

    // Verify
    TS_ASSERT_EQUALS(out.getOwner(), PLAYER_NR);
    TS_ASSERT_EQUALS(out.getHullType(), HULL_NR);
    TS_ASSERT_EQUALS(out.getName(), "Carola");
    TS_ASSERT_EQUALS(out.getFriendlyCode(), "abc");
    TS_ASSERT_EQUALS(out.getCrew(), 99);
    TS_ASSERT_EQUALS(out.getNumBeams(), 3);
    TS_ASSERT_EQUALS(out.getBeamType(), 4);
    TS_ASSERT_EQUALS(out.getEngineType(), 9);
    TS_ASSERT_EQUALS(out.getNumLaunchers(), 0);
    TS_ASSERT_EQUALS(out.getTorpedoType(), 0);
    TS_ASSERT_EQUALS(out.getNumBays(), 7);
    TS_ASSERT_EQUALS(out.getAmmo(), 100);        // set to cargo room because it's not known
    TS_ASSERT_EQUALS(out.getAggressiveness(), 7);
    TS_ASSERT_EQUALS(out.getFlags(), game::sim::Ship::fl_CommanderSet | game::sim::Ship::fl_Commander);
}

/** Test copy to regular ship. */
void
TestGameSimTransfer::testCopyToShip()
{
    // Environment
    UnitScoreDefinitionList scoreDefinitions;
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
    Transfer tr(scoreDefinitions, t.shipList(), t.config(), h, tx);
    TS_ASSERT_EQUALS(tr.copyShipToGame(out, in, t.universe()), true);

    // Verify changes
    TS_ASSERT_EQUALS(out.getName(), "Carola");
    TS_ASSERT_EQUALS(out.getFriendlyCode().orElse(""), "abc");
    TS_ASSERT_EQUALS(out.getMission().orElse(-1), int(game::spec::Mission::msn_Kill));
}

/** Test copy to mismatching ship. */
void
TestGameSimTransfer::testCopyToMismatchingShip()
{
    // Environment
    UnitScoreDefinitionList scoreDefinitions;
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
    Transfer tr(scoreDefinitions, t.shipList(), t.config(), h, tx);
    TS_ASSERT_EQUALS(tr.copyShipToGame(out, in, t.universe()), false);
}

/** Test copy to regular ship with fighter transfer. */
void
TestGameSimTransfer::testCopyToShipWithFighters()
{
    // Environment
    UnitScoreDefinitionList scoreDefinitions;
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
    Transfer tr(scoreDefinitions, t.shipList(), t.config(), h, tx);
    TS_ASSERT_EQUALS(tr.copyShipToGame(out, in, t.universe()), true);

    // Verify
    TS_ASSERT_EQUALS(pl.getCargo(Element::Fighters).orElse(0), 20);
    TS_ASSERT_EQUALS(out.getAmmo().orElse(0), 60);
}

/** Test copy to regular ship with torpedo transfer. */
void
TestGameSimTransfer::testCopyToShipWithTorps()
{
    // Environment
    UnitScoreDefinitionList scoreDefinitions;
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
    Transfer tr(scoreDefinitions, t.shipList(), t.config(), h, tx);
    TS_ASSERT_EQUALS(tr.copyShipToGame(out, in, t.universe()), true);

    // Verify
    TS_ASSERT_EQUALS(pl.getCargo(Element::fromTorpedoType(TORP_ID)).orElse(-1), 25);
    TS_ASSERT_EQUALS(out.getAmmo().orElse(0), 40);
}

/** Test copy from an empty planet. */
void
TestGameSimTransfer::testCopyFromEmptyPlanet()
{
    // Environment
    UnitScoreDefinitionList scoreDefinitions;
    NullTranslator tx;
    HostVersion h(HostVersion::Host, MKVERSION(3,22,0));
    SimpleTurn t;
    Planet* in = t.universe().planets().create(77);

    // Test
    Transfer tr(scoreDefinitions, t.shipList(), t.config(), h, tx);
    game::sim::Planet out;
    TS_ASSERT_EQUALS(tr.copyPlanetFromGame(out, *in), false);
}

/** Test copy from a regular planet. */
void
TestGameSimTransfer::testCopyFromPlanet()
{
    // Environment
    UnitScoreDefinitionList scoreDefinitions;
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
    Transfer tr(scoreDefinitions, t.shipList(), t.config(), h, tx);
    game::sim::Planet out;
    TS_ASSERT_EQUALS(tr.copyPlanetFromGame(out, in), true);

    // Verify
    TS_ASSERT_EQUALS(out.getOwner(), PLAYER_NR);
    TS_ASSERT_EQUALS(out.getName(), "Florida");
    TS_ASSERT_EQUALS(out.getFriendlyCode(), "efg");
    TS_ASSERT_EQUALS(out.getDefense(), 61);            // from colonists
    TS_ASSERT_EQUALS(out.getBaseBeamTech(), 0);
    TS_ASSERT_EQUALS(out.getBaseTorpedoTech(), 0);
    TS_ASSERT_EQUALS(out.getNumBaseFighters(), 0);
    TS_ASSERT_EQUALS(out.getFlags(), 0);
}

/** Test copy from a starbase. */
void
TestGameSimTransfer::testCopyFromBase()
{
    // Environment
    UnitScoreDefinitionList scoreDefinitions;
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
    Transfer tr(scoreDefinitions, t.shipList(), t.config(), h, tx);
    game::sim::Planet out;
    TS_ASSERT_EQUALS(tr.copyPlanetFromGame(out, in), true);

    // Verify
    TS_ASSERT_EQUALS(out.getOwner(), PLAYER_NR);
    TS_ASSERT_EQUALS(out.getName(), "Cuba");
    TS_ASSERT_EQUALS(out.getFriendlyCode(), "pqr");
    TS_ASSERT_EQUALS(out.getDefense(), 20);
    TS_ASSERT_EQUALS(out.getBaseBeamTech(), 4);
    TS_ASSERT_EQUALS(out.getBaseTorpedoTech(), 7);
    TS_ASSERT_EQUALS(out.getNumBaseFighters(), 30);
    TS_ASSERT_EQUALS(out.getBaseDefense(), 120);
    TS_ASSERT_EQUALS(out.getFlags(), 0);
}

/** Test copy to a regular planet. */
void
TestGameSimTransfer::testCopyToPlanet()
{
    // Environment
    UnitScoreDefinitionList scoreDefinitions;
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
    Transfer tr(scoreDefinitions, t.shipList(), t.config(), h, tx);
    game::sim::Planet in;
    in.setId(PLANET_ID);
    in.setOwner(PLAYER_NR);
    in.setFriendlyCode("hij");
    TS_ASSERT_EQUALS(tr.copyPlanetToGame(out, in), true);

    // Verify
    TS_ASSERT_EQUALS(out.getFriendlyCode().orElse(""), "hij");
}

/** Test copy to a mismatching planet. */
void
TestGameSimTransfer::testCopyToMismatchingPlanet()
{
    // Environment
    UnitScoreDefinitionList scoreDefinitions;
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
    Transfer tr(scoreDefinitions, t.shipList(), t.config(), h, tx);
    game::sim::Planet in;
    in.setId(PLANET_ID);
    in.setOwner(PLAYER_NR+1);
    in.setFriendlyCode("hij");
    TS_ASSERT_EQUALS(tr.copyPlanetToGame(out, in), false);

    // Verify
    TS_ASSERT_EQUALS(out.getFriendlyCode().orElse(""), "efg");
}
