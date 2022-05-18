/**
  *  \file u/t_game_map_universe.cpp
  *  \brief Test for game::map::Universe
  */

#include "game/map/universe.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/map/configuration.hpp"
#include "game/test/interpreterinterface.hpp"

using game::Reference;
using game::config::HostConfiguration;
using game::map::Point;
using game::map::Universe;

/** Test basics. */
void
TestGameMapUniverse::testBasics()
{
    Universe u;
    const Universe& cu = u;

    // Accessors
    TS_ASSERT_EQUALS(&u.ships(),         &cu.ships());
    TS_ASSERT_EQUALS(&u.playedShips(),   &cu.playedShips());
    TS_ASSERT_EQUALS(&u.planets(),       &cu.planets());
    TS_ASSERT_EQUALS(&u.playedPlanets(), &cu.playedPlanets());
    TS_ASSERT_EQUALS(&u.playedBases(),   &cu.playedBases());
    TS_ASSERT_EQUALS(&u.fleets(),        &cu.fleets());
    TS_ASSERT_EQUALS(&u.ionStorms(),     &cu.ionStorms());
    TS_ASSERT_EQUALS(&u.ionStormType(),  &cu.ionStormType());
    TS_ASSERT_EQUALS(&u.minefields(),    &cu.minefields());
    TS_ASSERT_EQUALS(&u.ufos(),          &cu.ufos());
    TS_ASSERT_EQUALS(&u.explosions(),    &cu.explosions());
    TS_ASSERT_EQUALS(&u.drawings(),      &cu.drawings());

    TS_ASSERT(cu.getReverter() == 0);
    TS_ASSERT(u.getReverter() == 0);
}

/** Test getObject(). */
void
TestGameMapUniverse::testGetObject()
{
    // Create some objects
    Universe u;
    game::map::Ship* s12 = u.ships().create(12);
    game::map::Planet* p37 = u.planets().create(37);
    game::map::Minefield* m42 = u.minefields().create(42);
    game::map::IonStorm* i7 = u.ionStorms().create(7);
    game::map::Ufo* u51 = u.ufos().addUfo(51, 1, 2);

    const Universe& cu = u;

    // Query existing objects
    TS_ASSERT_EQUALS(u.getObject(Reference(Reference::Ship, 12)), s12);
    TS_ASSERT_EQUALS(cu.getObject(Reference(Reference::Ship, 12)), s12);

    TS_ASSERT_EQUALS(u.getObject(Reference(Reference::Planet, 37)), p37);
    TS_ASSERT_EQUALS(cu.getObject(Reference(Reference::Planet, 37)), p37);
    TS_ASSERT_EQUALS(u.getObject(Reference(Reference::Starbase, 37)), p37);
    TS_ASSERT_EQUALS(cu.getObject(Reference(Reference::Starbase, 37)), p37);

    TS_ASSERT_EQUALS(u.getObject(Reference(Reference::Minefield, 42)), m42);
    TS_ASSERT_EQUALS(cu.getObject(Reference(Reference::Minefield, 42)), m42);

    TS_ASSERT_EQUALS(u.getObject(Reference(Reference::Storm, 7)), i7);
    TS_ASSERT_EQUALS(cu.getObject(Reference(Reference::Storm, 7)), i7);

    TS_ASSERT_EQUALS(u.getObject(Reference(Reference::Ufo, 51)), u51);
    TS_ASSERT_EQUALS(cu.getObject(Reference(Reference::Ufo, 51)), u51);

    // Invalid references
    TS_ASSERT(u.getObject(Reference(Reference::Ship, 99)) == 0);
    TS_ASSERT(u.getObject(Reference(Reference::Planet, 99)) == 0);
    TS_ASSERT(u.getObject(Reference(Reference::Starbase, 99)) == 0);
    TS_ASSERT(u.getObject(Reference(Reference::Minefield, 99)) == 0);
    TS_ASSERT(u.getObject(Reference(Reference::Storm, 99)) == 0);
    TS_ASSERT(u.getObject(Reference(Reference::Hull, 99)) == 0);
    TS_ASSERT(u.getObject(Reference(Reference::Beam, 99)) == 0);
    TS_ASSERT(u.getObject(Reference(Reference::Torpedo, 99)) == 0);
    TS_ASSERT(u.getObject(Reference(Reference::Engine, 99)) == 0);
    TS_ASSERT(u.getObject(Reference(Reference::Player, 99)) == 0);
    TS_ASSERT(u.getObject(Reference()) == 0);
}

/** Test find() functions. */
void
TestGameMapUniverse::testFind()
{
    // Some environment
    const game::map::Configuration mapConfig;
    game::HostVersion tim(game::HostVersion::Host, MKVERSION(3,22,0));
    game::HostVersion andrew(game::HostVersion::PHost, MKVERSION(3,2,5));
    HostConfiguration noWW, squareWW, roundWW;
    noWW[HostConfiguration::AllowGravityWells].set(0);
    squareWW[HostConfiguration::AllowGravityWells].set(1);
    squareWW[HostConfiguration::RoundGravityWells].set(0);
    roundWW[HostConfiguration::AllowGravityWells].set(1);
    roundWW[HostConfiguration::RoundGravityWells].set(1);
    game::spec::ShipList sl;
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    game::PlayerList pl;
    pl.create(4)->setName(game::Player::AdjectiveName, "fourish");
    pl.create(5)->setName(game::Player::AdjectiveName, "fiveish");

    game::test::InterpreterInterface iface;

    // Universe
    Universe u;
    game::map::Planet* p10 = u.planets().create(10);
    game::map::Planet* p20 = u.planets().create(20);
    game::map::Planet* p30 = u.planets().create(30);
    game::map::Planet* p40 = u.planets().create(40);
    p10->setPosition(Point(1000, 1000));    // base case
    p20->setPosition(Point(1000, 1003));    // close to #10
    p30->setPosition(Point(1010, 1000));    // just a planet
    p40->setPosition(Point(1020, 1000));    // just a planet
    p10->setName("Ten");
    p20->setName("Twenty");
    p30->setName("Thirty");
    p40->setName("Fourty");

    game::map::Ship* s5 = u.ships().create(5);
    game::map::Ship* s6 = u.ships().create(6);
    game::map::Ship* s7 = u.ships().create(7);
    game::map::Ship* s8 = u.ships().create(8);
    s5->addShipXYData(Point(1003, 1000), 4, 100, game::PlayerSet_t(5));     // in warp well of #10
    s6->addShipXYData(Point(1020, 1020), 4, 100, game::PlayerSet_t(5));     // just a ship
    s7->addShipXYData(Point(1020, 1020), 4, 100, game::PlayerSet_t(5));     // same position as ship #6
    s8->addShipXYData(Point(1020, 1000), 4, 100, game::PlayerSet_t(5));     // same position as planet #40
    s5->setName("Five");
    s6->setName("Six");
    s7->setName("Seven");
    s8->setName("Eight");

    u.postprocess(game::PlayerSet_t(5), game::PlayerSet_t(5), game::map::Object::Playable, mapConfig, tim, noWW, 7, sl, tx, log);

    // findPlanetAt/1
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1010, 1000)), 30);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1020, 1020)), 0);

    // findPlanetAt/5
    // - exact position, all combinations
    //   (note that squareWW, tim is not a valid combination)
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1010, 1000), false, mapConfig, noWW,     tim),    30);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1010, 1000), true,  mapConfig, noWW,     tim),    30);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1010, 1000), false, mapConfig, roundWW,  tim),    30);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1010, 1000), true,  mapConfig, roundWW,  tim),    30);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1010, 1000), false, mapConfig, noWW,     andrew), 30);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1010, 1000), true,  mapConfig, noWW,     andrew), 30);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1010, 1000), false, mapConfig, roundWW,  andrew), 30);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1010, 1000), true,  mapConfig, roundWW,  andrew), 30);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1010, 1000), false, mapConfig, squareWW, andrew), 30);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1010, 1000), true,  mapConfig, squareWW, andrew), 30);

    // - inexact position, all combinations
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1013, 1000), false, mapConfig, noWW,     tim),     0);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1013, 1000), true,  mapConfig, noWW,     tim),     0);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1013, 1000), false, mapConfig, roundWW,  tim),     0);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1013, 1000), true,  mapConfig, roundWW,  tim),    30);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1013, 1000), false, mapConfig, noWW,     andrew),  0);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1013, 1000), true,  mapConfig, noWW,     andrew),  0);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1013, 1000), false, mapConfig, roundWW,  andrew),  0);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1013, 1000), true,  mapConfig, roundWW,  andrew), 30);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1013, 1000), false, mapConfig, squareWW, andrew),  0);
    TS_ASSERT_EQUALS(u.findPlanetAt(Point(1013, 1000), true,  mapConfig, squareWW, andrew), 30);

    // findGravityPlanetAt
    // - inexact position testcases
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1013, 1000), mapConfig, noWW,     tim),     0);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1013, 1000), mapConfig, roundWW,  tim),    30);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1013, 1000), mapConfig, noWW,     andrew),  0);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1013, 1000), mapConfig, roundWW,  andrew), 30);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1013, 1000), mapConfig, squareWW, andrew), 30);

    // - outside round WW
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1013, 1003), mapConfig, noWW,     tim),     0);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1013, 1003), mapConfig, roundWW,  tim),     0);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1013, 1003), mapConfig, noWW,     andrew),  0);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1013, 1003), mapConfig, roundWW,  andrew),  0);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1013, 1003), mapConfig, squareWW, andrew), 30);

    // - warp-slide usecase
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(999, 999), mapConfig, noWW,     tim),     0);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(999, 999), mapConfig, roundWW,  tim),    20); // warp slide
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(999, 999), mapConfig, noWW,     andrew),  0);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(999, 999), mapConfig, roundWW,  andrew), 10);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(999, 999), mapConfig, squareWW, andrew), 10);

    // - in two warp wells
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1001, 1001), mapConfig, noWW,     tim),     0);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1001, 1001), mapConfig, roundWW,  tim),    20);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1001, 1001), mapConfig, noWW,     andrew),  0);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1001, 1001), mapConfig, roundWW,  andrew), 20);
    TS_ASSERT_EQUALS(u.findGravityPlanetAt(Point(1001, 1001), mapConfig, squareWW, andrew), 20);

    // findFirstShipAt
    TS_ASSERT_EQUALS(u.findFirstShipAt(Point(1000, 1000)), 0);
    TS_ASSERT_EQUALS(u.findFirstShipAt(Point(1020, 1020)), 6);

    // findLocationName
    // - planet
    TS_ASSERT_EQUALS(u.findLocationName(Point(1000, 1000), 0,                                           mapConfig, roundWW, andrew, tx), "Ten (#10)");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1000, 1000), Universe::NameVerbose,                       mapConfig, roundWW, andrew, tx), "Ten (Planet #10)");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1000, 1000), Universe::NameOrbit,                         mapConfig, roundWW, andrew, tx), "Orbit of Ten (#10)");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1000, 1000), Universe::NameVerbose | Universe::NameOrbit, mapConfig, roundWW, andrew, tx), "Orbit of Ten (Planet #10)");

    // - deep space
    TS_ASSERT_EQUALS(u.findLocationName(Point(700, 700), 0, mapConfig, roundWW, andrew, tx), "(700,700)");
    TS_ASSERT_EQUALS(u.findLocationName(Point(700, 700), Universe::NameVerbose, mapConfig, roundWW, andrew, tx), "Deep Space (700,700)");
    TS_ASSERT_EQUALS(u.findLocationName(Point(700, 700), Universe::NameNoSpace, mapConfig, roundWW, andrew, tx), "");

    // - gravity
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), 0,                                           mapConfig, roundWW, andrew, tx), "(1003,1000)");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameVerbose,                       mapConfig, roundWW, andrew, tx), "Deep Space (1003,1000)");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameOrbit,                         mapConfig, roundWW, andrew, tx), "(1003,1000)");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameVerbose | Universe::NameOrbit, mapConfig, roundWW, andrew, tx), "Deep Space (1003,1000)");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameGravity,                                               mapConfig, roundWW, andrew, tx), "near Ten (#10)");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameGravity | Universe::NameVerbose,                       mapConfig, roundWW, andrew, tx), "near Ten (Planet #10)");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameGravity | Universe::NameOrbit,                         mapConfig, roundWW, andrew, tx), "near Ten (#10)");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameGravity | Universe::NameVerbose | Universe::NameOrbit, mapConfig, roundWW, andrew, tx), "near Ten (Planet #10)");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameShips | 0,                                           mapConfig, roundWW, andrew, tx), "Ship #5: Five");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameShips | Universe::NameVerbose,                       mapConfig, roundWW, andrew, tx), "Ship #5: Five");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameShips | Universe::NameOrbit,                         mapConfig, roundWW, andrew, tx), "Ship #5: Five");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameShips | Universe::NameVerbose | Universe::NameOrbit, mapConfig, roundWW, andrew, tx), "Ship #5: Five");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameShips | Universe::NameGravity,                                               mapConfig, roundWW, andrew, tx), "Ship #5: Five");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameShips | Universe::NameGravity | Universe::NameVerbose,                       mapConfig, roundWW, andrew, tx), "Ship #5: Five");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameShips | Universe::NameGravity | Universe::NameOrbit,                         mapConfig, roundWW, andrew, tx), "Ship #5: Five");
    TS_ASSERT_EQUALS(u.findLocationName(Point(1003, 1000), Universe::NameShips | Universe::NameGravity | Universe::NameVerbose | Universe::NameOrbit, mapConfig, roundWW, andrew, tx), "Ship #5: Five");

    // findLocationUnitNames
    // - deep space
    TS_ASSERT_EQUALS(u.findLocationUnitNames(Point(999, 999), 5, pl, mapConfig, tx, iface), "");

    // - planet
    TS_ASSERT_EQUALS(u.findLocationUnitNames(Point(1000, 1000), 5, pl, mapConfig, tx, iface), "Planet #10: Ten");

    // - multiple ships (foreign/owner viewpoint)
    TS_ASSERT_EQUALS(u.findLocationUnitNames(Point(1020, 1020), 5, pl, mapConfig, tx, iface), "2 fourish ships");
    TS_ASSERT_EQUALS(u.findLocationUnitNames(Point(1020, 1020), 4, pl, mapConfig, tx, iface), "Ship #6: Six + 1 own ship");

    // - single ship (foreign/owner viewpoint)
    TS_ASSERT_EQUALS(u.findLocationUnitNames(Point(1003, 1000), 5, pl, mapConfig, tx, iface), "1 fourish ship");
    TS_ASSERT_EQUALS(u.findLocationUnitNames(Point(1003, 1000), 4, pl, mapConfig, tx, iface), "Ship #5: Five");

    // - ship and planet
    TS_ASSERT_EQUALS(u.findLocationUnitNames(Point(1020, 1000), 5, pl, mapConfig, tx, iface), "Planet #40: Fourty\n1 fourish ship");
    TS_ASSERT_EQUALS(u.findLocationUnitNames(Point(1020, 1000), 4, pl, mapConfig, tx, iface), "Planet #40: Fourty\nShip #8: Eight");
}

