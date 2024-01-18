/**
  *  \file test/game/map/cursorstest.cpp
  *  \brief Test for game::map::Cursors
  */

#include "game/map/cursors.hpp"

#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"

using game::Reference;
using game::map::Cursors;
using game::map::Point;

/** Test getCursorByNumber() / mapping to individual accessors. */
AFL_TEST("game.map.Cursors:getCursorByNumber", a)
{
    Cursors t;

    a.checkEqual("01. ShipScreen",    t.getCursorByNumber(Cursors::ShipScreen),    &t.currentShip());
    a.checkEqual("02. PlanetScreen",  t.getCursorByNumber(Cursors::PlanetScreen),  &t.currentPlanet());
    a.checkEqual("03. BaseScreen",    t.getCursorByNumber(Cursors::BaseScreen),    &t.currentBase());
    a.checkEqual("04. HistoryScreen", t.getCursorByNumber(Cursors::HistoryScreen), &t.currentHistoryShip());
    a.checkEqual("05. FleetScreen",   t.getCursorByNumber(Cursors::FleetScreen),   &t.currentFleet());
    a.checkNull ("06. AllShips",      t.getCursorByNumber(Cursors::AllShips));
    a.checkNull ("07. AllPlanets",    t.getCursorByNumber(Cursors::AllPlanets));
    a.checkEqual("08. Ufos",          t.getCursorByNumber(Cursors::Ufos),          &t.currentUfo());
    a.checkEqual("09. IonStorms",     t.getCursorByNumber(Cursors::IonStorms),     &t.currentIonStorm());
    a.checkEqual("10. Minefields",    t.getCursorByNumber(Cursors::Minefields),    &t.currentMinefield());

    // Out of range
    a.checkNull("11. range", t.getCursorByNumber(-1));
    a.checkNull("12. range", t.getCursorByNumber(99999));
}

/** Test getTypeByNumber(). */
AFL_TEST("game.map.Cursors:getTypeByNumber", a)
{
    Cursors t;
    game::map::Universe univ;
    game::map::Configuration mapConfig;
    t.setUniverse(&univ, &mapConfig);

    a.checkEqual("01. ShipScreen",    t.getTypeByNumber(Cursors::ShipScreen),    &univ.playedShips());
    a.checkEqual("02. PlanetScreen",  t.getTypeByNumber(Cursors::PlanetScreen),  &univ.playedPlanets());
    a.checkEqual("03. BaseScreen",    t.getTypeByNumber(Cursors::BaseScreen),    &univ.playedBases());
    a.checkEqual("04. HistoryScreen", t.getTypeByNumber(Cursors::HistoryScreen), &univ.historyShips());
    a.checkEqual("05. FleetScreen",   t.getTypeByNumber(Cursors::FleetScreen),   &univ.fleets());
    a.checkEqual("06. AllShips",      t.getTypeByNumber(Cursors::AllShips),      &univ.allShips());
    a.checkEqual("07. AllPlanets",    t.getTypeByNumber(Cursors::AllPlanets),    &univ.allPlanets());
    a.checkEqual("08. Ufos",          t.getTypeByNumber(Cursors::Ufos),          &univ.ufos());
    a.checkEqual("09. IonStormsa",    t.getTypeByNumber(Cursors::IonStorms),     &univ.ionStormType());
    a.checkEqual("10. Minefields",    t.getTypeByNumber(Cursors::Minefields),    &univ.minefields());

    // Out of range
    a.checkNull("11. range", t.getTypeByNumber(-1));
    a.checkNull("12. range", t.getTypeByNumber(99999));

    // Null universe
    t.setUniverse(0, 0);
    a.checkNull("21. ShipScreen",    t.getTypeByNumber(Cursors::ShipScreen)   );
    a.checkNull("22. PlanetScreen",  t.getTypeByNumber(Cursors::PlanetScreen) );
    a.checkNull("23. BaseScreen",    t.getTypeByNumber(Cursors::BaseScreen)   );
    a.checkNull("24. HistoryScreen", t.getTypeByNumber(Cursors::HistoryScreen));
    a.checkNull("25. FleetScreen",   t.getTypeByNumber(Cursors::FleetScreen)  );
    a.checkNull("26. AllShips",      t.getTypeByNumber(Cursors::AllShips)     );
    a.checkNull("27. AllPlanets",    t.getTypeByNumber(Cursors::AllPlanets)   );
    a.checkNull("28. Ufos",          t.getTypeByNumber(Cursors::Ufos)         );
    a.checkNull("29. IonStormsa",    t.getTypeByNumber(Cursors::IonStorms)    );
    a.checkNull("30. Minefields",    t.getTypeByNumber(Cursors::Minefields)   );
}

/** Test setUniverse().
    In particular, after a universe change, cursors adapt. */
AFL_TEST("game.map.Cursors:setUniverse", a)
{
    // Environment: three universes
    game::map::Universe u1;
    u1.ufos().addUfo(100, 1, 1)->setPosition(Point(1000, 1000));

    game::map::Universe u2;
    u2.ufos().addUfo(100, 1, 1)->setPosition(Point(1200, 1000));

    game::map::Universe u3;
    u3.ufos().addUfo(200, 1, 1)->setPosition(Point(2000, 1000));

    game::map::Configuration mapConfig;

    // Test object
    Cursors t;

    // Initial situation: ufo 100 selected on Ufo cursor
    t.setUniverse(&u1, &mapConfig);
    a.checkEqual("01. id",  t.currentUfo().getCurrentObject()->getId(), 100);
    a.checkEqual("02. pos", t.currentUfo().getCurrentObject()->getPosition().orElse(Point()), Point(1000, 1000));

    t.location().set(game::Reference(game::Reference::Ufo, 100));
    a.checkEqual("11. pos", t.location().getPosition().orElse(Point()), Point(1000, 1000));

    // Select another universe. selections must adapt.
    t.setUniverse(&u2, &mapConfig);
    a.checkEqual("21. id",  t.currentUfo().getCurrentObject()->getId(), 100);
    a.checkEqual("22. pos", t.currentUfo().getCurrentObject()->getPosition().orElse(Point()), Point(1200, 1000));
    a.checkEqual("23. pos", t.location().getPosition().orElse(Point()), Point(1200, 1000));

    // Select universe where object does not exist. New object selected on cursor, Location loses object lock and remains at position.
    t.setUniverse(&u3, &mapConfig);
    a.checkEqual("31. id",  t.currentUfo().getCurrentObject()->getId(), 200);
    a.checkEqual("32. pos", t.currentUfo().getCurrentObject()->getPosition().orElse(Point()), Point(2000, 1000));
    a.checkEqual("33. pos", t.location().getPosition().orElse(Point()), Point(1200, 1000));
}

/** Test getReferenceTypeByNumber(). */
AFL_TEST("game.map.Cursors:getReferenceTypeByNumber", a)
{
    a.checkEqual("01. ShipScreen",    Cursors::getReferenceTypeByNumber(Cursors::ShipScreen),    Reference::Ship);
    a.checkEqual("02. PlanetScreen",  Cursors::getReferenceTypeByNumber(Cursors::PlanetScreen),  Reference::Planet);
    a.checkEqual("03. BaseScreen",    Cursors::getReferenceTypeByNumber(Cursors::BaseScreen),    Reference::Starbase);
    a.checkEqual("04. HistoryScreen", Cursors::getReferenceTypeByNumber(Cursors::HistoryScreen), Reference::Ship);
    a.checkEqual("05. FleetScreen",   Cursors::getReferenceTypeByNumber(Cursors::FleetScreen),   Reference::Ship);
    a.checkEqual("06. AllShips",      Cursors::getReferenceTypeByNumber(Cursors::AllShips),      Reference::Ship);
    a.checkEqual("07. AllPlanets",    Cursors::getReferenceTypeByNumber(Cursors::AllPlanets),    Reference::Planet);
    a.checkEqual("08. Ufos",          Cursors::getReferenceTypeByNumber(Cursors::Ufos),          Reference::Ufo);
    a.checkEqual("09. IonStormsa",    Cursors::getReferenceTypeByNumber(Cursors::IonStorms),     Reference::IonStorm);
    a.checkEqual("10. Minefields",    Cursors::getReferenceTypeByNumber(Cursors::Minefields),    Reference::Minefield);

    // Out of range
    a.checkEqual("11. range", Cursors::getReferenceTypeByNumber(-1), Reference::Null);
    a.checkEqual("12. range", Cursors::getReferenceTypeByNumber(99999), Reference::Null);
}
