/**
  *  \file test/game/map/universetest.cpp
  *  \brief Test for game::map::Universe
  */

#include "game/map/universe.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/test/interpreterinterface.hpp"

using game::Reference;
using game::config::HostConfiguration;
using game::map::Point;
using game::map::Universe;

/** Test basics. */
AFL_TEST("game.map.Universe:basics", a)
{
    Universe u;
    const Universe& cu = u;

    // Accessors
    a.checkEqual("01. ships",         &u.ships(),         &cu.ships());
    a.checkEqual("02. playedShips",   &u.playedShips(),   &cu.playedShips());
    a.checkEqual("03. allShips",      &u.allShips(),      &cu.allShips());
    a.checkEqual("04. planets",       &u.planets(),       &cu.planets());
    a.checkEqual("05. playedPlanets", &u.playedPlanets(), &cu.playedPlanets());
    a.checkEqual("06. playedBases",   &u.playedBases(),   &cu.playedBases());
    a.checkEqual("07. allPlanets",    &u.allPlanets(),    &cu.allPlanets());
    a.checkEqual("08. fleets",        &u.fleets(),        &cu.fleets());
    a.checkEqual("09. ionStorms",     &u.ionStorms(),     &cu.ionStorms());
    a.checkEqual("10. ionStormType",  &u.ionStormType(),  &cu.ionStormType());
    a.checkEqual("11. minefields",    &u.minefields(),    &cu.minefields());
    a.checkEqual("12. ufos",          &u.ufos(),          &cu.ufos());
    a.checkEqual("13. explosions",    &u.explosions(),    &cu.explosions());
    a.checkEqual("14. drawings",      &u.drawings(),      &cu.drawings());

    a.checkNull("21. getReverter", cu.getReverter());
    a.checkNull("22. getReverter", u.getReverter());
}

/** Test getObject(). */
AFL_TEST("game.map.Universe:getObject", a)
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
    a.checkEqual("01. Ship", u.getObject(Reference(Reference::Ship, 12)), s12);
    a.checkEqual("02. Ship", cu.getObject(Reference(Reference::Ship, 12)), s12);

    a.checkEqual("11. Planet", u.getObject(Reference(Reference::Planet, 37)), p37);
    a.checkEqual("12. Planet", cu.getObject(Reference(Reference::Planet, 37)), p37);
    a.checkEqual("13. Starbase", u.getObject(Reference(Reference::Starbase, 37)), p37);
    a.checkEqual("14. Starbase", cu.getObject(Reference(Reference::Starbase, 37)), p37);

    a.checkEqual("21. Minefield", u.getObject(Reference(Reference::Minefield, 42)), m42);
    a.checkEqual("22. Minefield", cu.getObject(Reference(Reference::Minefield, 42)), m42);

    a.checkEqual("31. IonStorm", u.getObject(Reference(Reference::IonStorm, 7)), i7);
    a.checkEqual("32. IonStorm", cu.getObject(Reference(Reference::IonStorm, 7)), i7);

    a.checkEqual("41. Ufo", u.getObject(Reference(Reference::Ufo, 51)), u51);
    a.checkEqual("42. Ufo", cu.getObject(Reference(Reference::Ufo, 51)), u51);

    // Invalid references
    a.checkNull("51. Ship",      u.getObject(Reference(Reference::Ship, 99)));
    a.checkNull("52. Planet",    u.getObject(Reference(Reference::Planet, 99)));
    a.checkNull("53. Starbase",  u.getObject(Reference(Reference::Starbase, 99)));
    a.checkNull("54. Minefield", u.getObject(Reference(Reference::Minefield, 99)));
    a.checkNull("55. IonStorm",  u.getObject(Reference(Reference::IonStorm, 99)));
    a.checkNull("56. Hull",      u.getObject(Reference(Reference::Hull, 99)));
    a.checkNull("57. Beam",      u.getObject(Reference(Reference::Beam, 99)));
    a.checkNull("58. Torpedo",   u.getObject(Reference(Reference::Torpedo, 99)));
    a.checkNull("59. Engine",    u.getObject(Reference(Reference::Engine, 99)));
    a.checkNull("60. Player",    u.getObject(Reference(Reference::Player, 99)));
    a.checkNull("61. null",      u.getObject(Reference()));
}

/** Test find() functions. */
AFL_TEST("game.map.Universe:find", a)
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
    a.checkEqual("01. findPlanetAt", u.findPlanetAt(Point(1010, 1000)), 30);
    a.checkEqual("02. findPlanetAt", u.findPlanetAt(Point(1020, 1020)), 0);

    // findPlanetAt/5
    // - exact position, all combinations
    //   (note that squareWW, tim is not a valid combination)
    a.checkEqual("11. findPlanetAt", u.findPlanetAt(Point(1010, 1000), false, mapConfig, noWW,     tim),    30);
    a.checkEqual("12. findPlanetAt", u.findPlanetAt(Point(1010, 1000), true,  mapConfig, noWW,     tim),    30);
    a.checkEqual("13. findPlanetAt", u.findPlanetAt(Point(1010, 1000), false, mapConfig, roundWW,  tim),    30);
    a.checkEqual("14. findPlanetAt", u.findPlanetAt(Point(1010, 1000), true,  mapConfig, roundWW,  tim),    30);
    a.checkEqual("15. findPlanetAt", u.findPlanetAt(Point(1010, 1000), false, mapConfig, noWW,     andrew), 30);
    a.checkEqual("16. findPlanetAt", u.findPlanetAt(Point(1010, 1000), true,  mapConfig, noWW,     andrew), 30);
    a.checkEqual("17. findPlanetAt", u.findPlanetAt(Point(1010, 1000), false, mapConfig, roundWW,  andrew), 30);
    a.checkEqual("18. findPlanetAt", u.findPlanetAt(Point(1010, 1000), true,  mapConfig, roundWW,  andrew), 30);
    a.checkEqual("19. findPlanetAt", u.findPlanetAt(Point(1010, 1000), false, mapConfig, squareWW, andrew), 30);
    a.checkEqual("20. findPlanetAt", u.findPlanetAt(Point(1010, 1000), true,  mapConfig, squareWW, andrew), 30);

    // - inexact position, all combinations
    a.checkEqual("21. findPlanetAt", u.findPlanetAt(Point(1013, 1000), false, mapConfig, noWW,     tim),     0);
    a.checkEqual("22. findPlanetAt", u.findPlanetAt(Point(1013, 1000), true,  mapConfig, noWW,     tim),     0);
    a.checkEqual("23. findPlanetAt", u.findPlanetAt(Point(1013, 1000), false, mapConfig, roundWW,  tim),     0);
    a.checkEqual("24. findPlanetAt", u.findPlanetAt(Point(1013, 1000), true,  mapConfig, roundWW,  tim),    30);
    a.checkEqual("25. findPlanetAt", u.findPlanetAt(Point(1013, 1000), false, mapConfig, noWW,     andrew),  0);
    a.checkEqual("26. findPlanetAt", u.findPlanetAt(Point(1013, 1000), true,  mapConfig, noWW,     andrew),  0);
    a.checkEqual("27. findPlanetAt", u.findPlanetAt(Point(1013, 1000), false, mapConfig, roundWW,  andrew),  0);
    a.checkEqual("28. findPlanetAt", u.findPlanetAt(Point(1013, 1000), true,  mapConfig, roundWW,  andrew), 30);
    a.checkEqual("29. findPlanetAt", u.findPlanetAt(Point(1013, 1000), false, mapConfig, squareWW, andrew),  0);
    a.checkEqual("30. findPlanetAt", u.findPlanetAt(Point(1013, 1000), true,  mapConfig, squareWW, andrew), 30);

    // findGravityPlanetAt
    // - inexact position testcases
    a.checkEqual("31. findGravityPlanetAt", u.findGravityPlanetAt(Point(1013, 1000), mapConfig, noWW,     tim),     0);
    a.checkEqual("32. findGravityPlanetAt", u.findGravityPlanetAt(Point(1013, 1000), mapConfig, roundWW,  tim),    30);
    a.checkEqual("33. findGravityPlanetAt", u.findGravityPlanetAt(Point(1013, 1000), mapConfig, noWW,     andrew),  0);
    a.checkEqual("34. findGravityPlanetAt", u.findGravityPlanetAt(Point(1013, 1000), mapConfig, roundWW,  andrew), 30);
    a.checkEqual("35. findGravityPlanetAt", u.findGravityPlanetAt(Point(1013, 1000), mapConfig, squareWW, andrew), 30);

    // - outside round WW
    a.checkEqual("41. findGravityPlanetAt", u.findGravityPlanetAt(Point(1013, 1003), mapConfig, noWW,     tim),     0);
    a.checkEqual("42. findGravityPlanetAt", u.findGravityPlanetAt(Point(1013, 1003), mapConfig, roundWW,  tim),     0);
    a.checkEqual("43. findGravityPlanetAt", u.findGravityPlanetAt(Point(1013, 1003), mapConfig, noWW,     andrew),  0);
    a.checkEqual("44. findGravityPlanetAt", u.findGravityPlanetAt(Point(1013, 1003), mapConfig, roundWW,  andrew),  0);
    a.checkEqual("45. findGravityPlanetAt", u.findGravityPlanetAt(Point(1013, 1003), mapConfig, squareWW, andrew), 30);

    // - warp-slide usecase
    a.checkEqual("51. findGravityPlanetAt", u.findGravityPlanetAt(Point(999, 999), mapConfig, noWW,     tim),     0);
    a.checkEqual("52. findGravityPlanetAt", u.findGravityPlanetAt(Point(999, 999), mapConfig, roundWW,  tim),    20); // warp slide
    a.checkEqual("53. findGravityPlanetAt", u.findGravityPlanetAt(Point(999, 999), mapConfig, noWW,     andrew),  0);
    a.checkEqual("54. findGravityPlanetAt", u.findGravityPlanetAt(Point(999, 999), mapConfig, roundWW,  andrew), 10);
    a.checkEqual("55. findGravityPlanetAt", u.findGravityPlanetAt(Point(999, 999), mapConfig, squareWW, andrew), 10);

    // - in two warp wells
    a.checkEqual("61. findGravityPlanetAt", u.findGravityPlanetAt(Point(1001, 1001), mapConfig, noWW,     tim),     0);
    a.checkEqual("62. findGravityPlanetAt", u.findGravityPlanetAt(Point(1001, 1001), mapConfig, roundWW,  tim),    20);
    a.checkEqual("63. findGravityPlanetAt", u.findGravityPlanetAt(Point(1001, 1001), mapConfig, noWW,     andrew),  0);
    a.checkEqual("64. findGravityPlanetAt", u.findGravityPlanetAt(Point(1001, 1001), mapConfig, roundWW,  andrew), 20);
    a.checkEqual("65. findGravityPlanetAt", u.findGravityPlanetAt(Point(1001, 1001), mapConfig, squareWW, andrew), 20);

    // findFirstShipAt
    a.checkEqual("71. findFirstShipAt", u.findFirstShipAt(Point(1000, 1000)), 0);
    a.checkEqual("72. findFirstShipAt", u.findFirstShipAt(Point(1020, 1020)), 6);

    // findLocationName
    // - planet
    a.checkEqual("81. findLocationName", u.findLocationName(Point(1000, 1000), 0,                                           mapConfig, roundWW, andrew, tx), "Ten (#10)");
    a.checkEqual("82. findLocationName", u.findLocationName(Point(1000, 1000), Universe::NameVerbose,                       mapConfig, roundWW, andrew, tx), "Ten (Planet #10)");
    a.checkEqual("83. findLocationName", u.findLocationName(Point(1000, 1000), Universe::NameOrbit,                         mapConfig, roundWW, andrew, tx), "Orbit of Ten (#10)");
    a.checkEqual("84. findLocationName", u.findLocationName(Point(1000, 1000), Universe::NameVerbose | Universe::NameOrbit, mapConfig, roundWW, andrew, tx), "Orbit of Ten (Planet #10)");

    // - deep space
    a.checkEqual("91. findLocationName", u.findLocationName(Point(700, 700), 0, mapConfig, roundWW, andrew, tx), "(700,700)");
    a.checkEqual("92. findLocationName", u.findLocationName(Point(700, 700), Universe::NameVerbose, mapConfig, roundWW, andrew, tx), "Deep Space (700,700)");
    a.checkEqual("93. findLocationName", u.findLocationName(Point(700, 700), Universe::NameNoSpace, mapConfig, roundWW, andrew, tx), "");

    // - gravity
    a.checkEqual("101. findLocationName", u.findLocationName(Point(1003, 1000), 0,                                           mapConfig, roundWW, andrew, tx), "(1003,1000)");
    a.checkEqual("102. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameVerbose,                       mapConfig, roundWW, andrew, tx), "Deep Space (1003,1000)");
    a.checkEqual("103. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameOrbit,                         mapConfig, roundWW, andrew, tx), "(1003,1000)");
    a.checkEqual("104. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameVerbose | Universe::NameOrbit, mapConfig, roundWW, andrew, tx), "Deep Space (1003,1000)");
    a.checkEqual("105. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameGravity,                                               mapConfig, roundWW, andrew, tx), "near Ten (#10)");
    a.checkEqual("106. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameGravity | Universe::NameVerbose,                       mapConfig, roundWW, andrew, tx), "near Ten (Planet #10)");
    a.checkEqual("107. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameGravity | Universe::NameOrbit,                         mapConfig, roundWW, andrew, tx), "near Ten (#10)");
    a.checkEqual("108. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameGravity | Universe::NameVerbose | Universe::NameOrbit, mapConfig, roundWW, andrew, tx), "near Ten (Planet #10)");
    a.checkEqual("109. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameShips | 0,                                           mapConfig, roundWW, andrew, tx), "Ship #5: Five");
    a.checkEqual("110. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameShips | Universe::NameVerbose,                       mapConfig, roundWW, andrew, tx), "Ship #5: Five");
    a.checkEqual("111. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameShips | Universe::NameOrbit,                         mapConfig, roundWW, andrew, tx), "Ship #5: Five");
    a.checkEqual("112. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameShips | Universe::NameVerbose | Universe::NameOrbit, mapConfig, roundWW, andrew, tx), "Ship #5: Five");
    a.checkEqual("113. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameShips | Universe::NameGravity,                                               mapConfig, roundWW, andrew, tx), "Ship #5: Five");
    a.checkEqual("114. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameShips | Universe::NameGravity | Universe::NameVerbose,                       mapConfig, roundWW, andrew, tx), "Ship #5: Five");
    a.checkEqual("115. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameShips | Universe::NameGravity | Universe::NameOrbit,                         mapConfig, roundWW, andrew, tx), "Ship #5: Five");
    a.checkEqual("116. findLocationName", u.findLocationName(Point(1003, 1000), Universe::NameShips | Universe::NameGravity | Universe::NameVerbose | Universe::NameOrbit, mapConfig, roundWW, andrew, tx), "Ship #5: Five");

    // findLocationUnitNames
    // - deep space
    a.checkEqual("121. findLocationUnitNames", u.findLocationUnitNames(Point(999, 999), 5, pl, mapConfig, tx, iface), "");

    // - planet
    a.checkEqual("131. findLocationUnitNames", u.findLocationUnitNames(Point(1000, 1000), 5, pl, mapConfig, tx, iface), "Planet #10: Ten");

    // - multiple ships (foreign/owner viewpoint)
    a.checkEqual("141. findLocationUnitNames", u.findLocationUnitNames(Point(1020, 1020), 5, pl, mapConfig, tx, iface), "2 fourish ships");
    a.checkEqual("142. findLocationUnitNames", u.findLocationUnitNames(Point(1020, 1020), 4, pl, mapConfig, tx, iface), "Ship #6: Six + 1 own ship");

    // - single ship (foreign/owner viewpoint)
    a.checkEqual("151. findLocationUnitNames", u.findLocationUnitNames(Point(1003, 1000), 5, pl, mapConfig, tx, iface), "1 fourish ship");
    a.checkEqual("152. findLocationUnitNames", u.findLocationUnitNames(Point(1003, 1000), 4, pl, mapConfig, tx, iface), "Ship #5: Five");

    // - ship and planet
    a.checkEqual("161. findLocationUnitNames", u.findLocationUnitNames(Point(1020, 1000), 5, pl, mapConfig, tx, iface), "Planet #40: Fourty\n1 fourish ship");
    a.checkEqual("162. findLocationUnitNames", u.findLocationUnitNames(Point(1020, 1000), 4, pl, mapConfig, tx, iface), "Planet #40: Fourty\nShip #8: Eight");
}
