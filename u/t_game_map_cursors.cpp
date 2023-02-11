/**
  *  \file u/t_game_map_cursors.cpp
  *  \brief Test for game::map::Cursors
  */

#include "game/map/cursors.hpp"

#include "t_game_map.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"

using game::Reference;
using game::map::Cursors;
using game::map::Point;

/** Test getCursorByNumber() / mapping to individual accessors. */
void
TestGameMapCursors::testGetCursorByNumber()
{
    Cursors t;

    TS_ASSERT_EQUALS(t.getCursorByNumber(Cursors::ShipScreen),    &t.currentShip());
    TS_ASSERT_EQUALS(t.getCursorByNumber(Cursors::PlanetScreen),  &t.currentPlanet());
    TS_ASSERT_EQUALS(t.getCursorByNumber(Cursors::BaseScreen),    &t.currentBase());
    TS_ASSERT_EQUALS(t.getCursorByNumber(Cursors::HistoryScreen), &t.currentHistoryShip());
    TS_ASSERT_EQUALS(t.getCursorByNumber(Cursors::FleetScreen),   &t.currentFleet());
    TS_ASSERT(t.getCursorByNumber(Cursors::AllShips) == 0);
    TS_ASSERT(t.getCursorByNumber(Cursors::AllPlanets) == 0);
    TS_ASSERT_EQUALS(t.getCursorByNumber(Cursors::Ufos),          &t.currentUfo());
    TS_ASSERT_EQUALS(t.getCursorByNumber(Cursors::IonStorms),     &t.currentIonStorm());
    TS_ASSERT_EQUALS(t.getCursorByNumber(Cursors::Minefields),    &t.currentMinefield());

    // Out of range
    TS_ASSERT(t.getCursorByNumber(-1) == 0);
    TS_ASSERT(t.getCursorByNumber(99999) == 0);
}

/** Test getTypeByNumber(). */
void
TestGameMapCursors::testGetTypeByNumber()
{
    Cursors t;
    game::map::Universe univ;
    game::map::Configuration mapConfig;
    t.setUniverse(&univ, &mapConfig);

    TS_ASSERT_EQUALS(t.getTypeByNumber(Cursors::ShipScreen),    &univ.playedShips());
    TS_ASSERT_EQUALS(t.getTypeByNumber(Cursors::PlanetScreen),  &univ.playedPlanets());
    TS_ASSERT_EQUALS(t.getTypeByNumber(Cursors::BaseScreen),    &univ.playedBases());
    TS_ASSERT_EQUALS(t.getTypeByNumber(Cursors::HistoryScreen), &univ.historyShips());
    TS_ASSERT_EQUALS(t.getTypeByNumber(Cursors::FleetScreen),   &univ.fleets());
    TS_ASSERT_EQUALS(t.getTypeByNumber(Cursors::AllShips),      &univ.allShips());
    TS_ASSERT_EQUALS(t.getTypeByNumber(Cursors::AllPlanets),    &univ.allPlanets());
    TS_ASSERT_EQUALS(t.getTypeByNumber(Cursors::Ufos),          &univ.ufos());
    TS_ASSERT_EQUALS(t.getTypeByNumber(Cursors::IonStorms),     &univ.ionStormType());
    TS_ASSERT_EQUALS(t.getTypeByNumber(Cursors::Minefields),    &univ.minefields());

    // Out of range
    TS_ASSERT(t.getTypeByNumber(-1) == 0);
    TS_ASSERT(t.getTypeByNumber(99999) == 0);

    // Null universe
    t.setUniverse(0, 0);
    TS_ASSERT(t.getTypeByNumber(Cursors::ShipScreen)    == 0);
    TS_ASSERT(t.getTypeByNumber(Cursors::PlanetScreen)  == 0);
    TS_ASSERT(t.getTypeByNumber(Cursors::BaseScreen)    == 0);
    TS_ASSERT(t.getTypeByNumber(Cursors::HistoryScreen) == 0);
    TS_ASSERT(t.getTypeByNumber(Cursors::FleetScreen)   == 0);
    TS_ASSERT(t.getTypeByNumber(Cursors::AllShips)      == 0);
    TS_ASSERT(t.getTypeByNumber(Cursors::AllPlanets)    == 0);
    TS_ASSERT(t.getTypeByNumber(Cursors::Ufos)          == 0);
    TS_ASSERT(t.getTypeByNumber(Cursors::IonStorms)     == 0);
    TS_ASSERT(t.getTypeByNumber(Cursors::Minefields)    == 0);
}

/** Test setUniverse().
    In particular, after a universe change, cursors adapt. */
void
TestGameMapCursors::testSetUniverse()
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
    TS_ASSERT_EQUALS(t.currentUfo().getCurrentObject()->getId(), 100);
    TS_ASSERT_EQUALS(t.currentUfo().getCurrentObject()->getPosition().orElse(Point()), Point(1000, 1000));

    t.location().set(game::Reference(game::Reference::Ufo, 100));
    TS_ASSERT_EQUALS(t.location().getPosition().orElse(Point()), Point(1000, 1000));

    // Select another universe. selections must adapt.
    t.setUniverse(&u2, &mapConfig);
    TS_ASSERT_EQUALS(t.currentUfo().getCurrentObject()->getId(), 100);
    TS_ASSERT_EQUALS(t.currentUfo().getCurrentObject()->getPosition().orElse(Point()), Point(1200, 1000));
    TS_ASSERT_EQUALS(t.location().getPosition().orElse(Point()), Point(1200, 1000));

    // Select universe where object does not exist. New object selected on cursor, Location loses object lock and remains at position.
    t.setUniverse(&u3, &mapConfig);
    TS_ASSERT_EQUALS(t.currentUfo().getCurrentObject()->getId(), 200);
    TS_ASSERT_EQUALS(t.currentUfo().getCurrentObject()->getPosition().orElse(Point()), Point(2000, 1000));
    TS_ASSERT_EQUALS(t.location().getPosition().orElse(Point()), Point(1200, 1000));
}

/** Test getReferenceTypeByNumber(). */
void
TestGameMapCursors::testGetReferenceTypeByNumber()
{
    TS_ASSERT_EQUALS(Cursors::getReferenceTypeByNumber(Cursors::ShipScreen),    Reference::Ship);
    TS_ASSERT_EQUALS(Cursors::getReferenceTypeByNumber(Cursors::PlanetScreen),  Reference::Planet);
    TS_ASSERT_EQUALS(Cursors::getReferenceTypeByNumber(Cursors::BaseScreen),    Reference::Starbase);
    TS_ASSERT_EQUALS(Cursors::getReferenceTypeByNumber(Cursors::HistoryScreen), Reference::Ship);
    TS_ASSERT_EQUALS(Cursors::getReferenceTypeByNumber(Cursors::FleetScreen),   Reference::Ship);
    TS_ASSERT_EQUALS(Cursors::getReferenceTypeByNumber(Cursors::AllShips),      Reference::Ship);
    TS_ASSERT_EQUALS(Cursors::getReferenceTypeByNumber(Cursors::AllPlanets),    Reference::Planet);
    TS_ASSERT_EQUALS(Cursors::getReferenceTypeByNumber(Cursors::Ufos),          Reference::Ufo);
    TS_ASSERT_EQUALS(Cursors::getReferenceTypeByNumber(Cursors::IonStorms),     Reference::IonStorm);
    TS_ASSERT_EQUALS(Cursors::getReferenceTypeByNumber(Cursors::Minefields),    Reference::Minefield);

    // Out of range
    TS_ASSERT_EQUALS(Cursors::getReferenceTypeByNumber(-1), Reference::Null);
    TS_ASSERT_EQUALS(Cursors::getReferenceTypeByNumber(99999), Reference::Null);
}

