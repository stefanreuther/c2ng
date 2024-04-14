/**
  *  \file test/game/test/simpleturntest.cpp
  *  \brief Test for game::test::SimpleTurn
  */

#include "game/test/simpleturn.hpp"

#include "afl/test/testrunner.hpp"

using game::HostVersion;
using game::map::Object;
using game::map::Planet;
using game::map::Point;
using game::map::Ship;

AFL_TEST("game.test.SimpleTurn", a)
{
    const int HULL_NR = 12;

    game::test::SimpleTurn testee;
    testee.setPosition(Point(2000, 2300));
    testee.setHull(HULL_NR);

    Ship& sh   = testee.addShip  (50, 5, Object::ReadOnly);
    Planet& pl = testee.addPlanet(30, 7, Object::Playable);
    Planet& ba = testee.addBase  (40, 7, Object::Playable);

    a.checkEqual("01. ship id",    sh.getId(), 50);
    a.checkEqual("02. planet id",  pl.getId(), 30);
    a.checkEqual("03. base id",    ba.getId(), 40);

    a.checkEqual("11. ship hull",  sh.getHull().orElse(0), HULL_NR);
    a.checkEqual("12. planet",     pl.hasBase(), false);
    a.checkEqual("13. base",       ba.hasBase(), true);

    a.checkEqual("21. ship pos",   sh.getPosition().orElse(Point()), Point(2000, 2300));
    a.checkEqual("22. planet pos", pl.getPosition().orElse(Point()), Point(2000, 2300));
    a.checkEqual("23. base pos",   ba.getPosition().orElse(Point()), Point(2000, 2300));

    a.checkEqual("31. ship own",   sh.getOwner().orElse(0), 5);
    a.checkEqual("32. planet own", pl.getOwner().orElse(0), 7);
    a.checkEqual("33. base own",   ba.getOwner().orElse(0), 7);

    // Connectivity
    a.checkNonNull("41. hull", testee.shipList().hulls().get(HULL_NR));
    a.checkEqual  ("42. univ", &testee.universe(), &testee.turn().universe());
    a.checkNonNull("43. interface", &testee.interface());
    a.checkNonNull("44. config",    &testee.config());
    a.checkNonNull("45. mapConfig", &testee.mapConfiguration());
    a.checkNonNull("46. version",   &testee.version());

    a.checkEqual("51. host kind", testee.version().getKind(), HostVersion::PHost);
}
