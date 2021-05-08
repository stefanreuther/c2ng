/**
  *  \file u/t_game_map_boundingbox.cpp
  *  \brief Test for game::map::BoundingBox
  */

#include "game/map/boundingbox.hpp"

#include "t_game_map.hpp"
#include "game/map/drawing.hpp"
#include "afl/sys/log.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/spec/shiplist.hpp"
#include "game/map/universe.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"

using game::map::Drawing;
using game::map::Point;

/** Test default initialisation. */
void
TestGameMapBoundingBox::testInit()
{
    game::map::BoundingBox t;
    TS_ASSERT_EQUALS(t.getMinimumCoordinates(), t.getMaximumCoordinates());
}

/** Test addPoint(). */
void
TestGameMapBoundingBox::testAddPoint()
{
    game::map::BoundingBox t;
    t.addPoint(Point(1000, 2000));
    TS_ASSERT_EQUALS(t.getMinimumCoordinates(), Point(1000, 2000));
    TS_ASSERT_EQUALS(t.getMaximumCoordinates(), Point(1001, 2001));

    t.addPoint(Point(1500, 1400));
    TS_ASSERT_EQUALS(t.getMinimumCoordinates(), Point(1000, 1400));
    TS_ASSERT_EQUALS(t.getMaximumCoordinates(), Point(1501, 2001));
}

/** Test addCircle(). */
void
TestGameMapBoundingBox::testAddCircle()
{
    game::map::BoundingBox t;
    t.addCircle(Point(1200, 1300), 30);

    TS_ASSERT_EQUALS(t.getMinimumCoordinates(), Point(1170, 1270));
    TS_ASSERT_EQUALS(t.getMaximumCoordinates(), Point(1231, 1331));
}

/** Test addDrawing().
    Verifies all drawing types. */
void
TestGameMapBoundingBox::testAddDrawing()
{
    // Line
    {
        game::map::BoundingBox t;
        Drawing d(Point(1000, 2000), Drawing::LineDrawing);
        d.setPos2(Point(1200, 1400));
        t.addDrawing(d);

        TS_ASSERT_EQUALS(t.getMinimumCoordinates(), Point(1000, 1400));
        TS_ASSERT_EQUALS(t.getMaximumCoordinates(), Point(1201, 2001));
    }

    // Rectangle
    {
        game::map::BoundingBox t;
        Drawing d(Point(1200, 1300), Drawing::RectangleDrawing);
        d.setPos2(Point(1400, 1500));
        t.addDrawing(d);

        TS_ASSERT_EQUALS(t.getMinimumCoordinates(), Point(1200, 1300));
        TS_ASSERT_EQUALS(t.getMaximumCoordinates(), Point(1401, 1501));
    }

    // Circle
    {
        game::map::BoundingBox t;
        Drawing d(Point(1100, 1500), Drawing::CircleDrawing);
        d.setCircleRadius(25);
        t.addDrawing(d);

        TS_ASSERT_EQUALS(t.getMinimumCoordinates(), Point(1075, 1475));
        TS_ASSERT_EQUALS(t.getMaximumCoordinates(), Point(1126, 1526));
    }

    // Marker
    {
        game::map::BoundingBox t;
        Drawing d(Point(1500, 1600), Drawing::MarkerDrawing);
        t.addDrawing(d);

        TS_ASSERT_EQUALS(t.getMinimumCoordinates(), Point(1490, 1590));
        TS_ASSERT_EQUALS(t.getMaximumCoordinates(), Point(1511, 1611));
    }
}

/** Test addUniverse().
    Test multiple universes, and prove for each that all objects are considered. */
void
TestGameMapBoundingBox::testAddUniverse()
{
    game::PlayerSet_t set(1);
    game::HostVersion host(game::HostVersion::Host, MKVERSION(3,22,44));
    game::config::HostConfiguration config;
    game::spec::ShipList shipList;
    shipList.hulls().create(1);
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    const int TURN = 10;

    // East: planet, west: ship. North/south borders determined by map config
    {
        game::map::Universe univ;
        TS_ASSERT_EQUALS(univ.config().getMinimumCoordinates(), Point(1000, 1000));
        TS_ASSERT_EQUALS(univ.config().getMaximumCoordinates(), Point(3000, 3000));

        game::map::Planet* p = univ.planets().create(10);
        p->setPosition(Point(500, 2000));
        p->setOwner(0);

        game::map::Ship* sh = univ.ships().create(20);
        sh->addShipXYData(Point(3400, 2000), 2, 500, set);

        univ.postprocess(set, set, game::map::Object::ReadOnly, host, config, TURN, shipList, tx, log);

        // Test
        game::map::BoundingBox t;
        t.addUniverse(univ);

        TS_ASSERT_EQUALS(t.getMinimumCoordinates(), Point( 500, 1000));
        TS_ASSERT_EQUALS(t.getMaximumCoordinates(), Point(3401, 3001));
    }

    // North: ion storm, south: minefield, east: drawing, west: ufo
    {
        game::map::Universe univ;
        game::map::IonStorm* st = univ.ionStorms().create(10);
        st->setPosition(Point(2000, 600));
        st->setRadius(250);
        st->setVoltage(20);

        game::map::Minefield* mf = univ.minefields().create(20);
        mf->addReport(Point(2000, 3400), 1, game::map::Minefield::IsMine, game::map::Minefield::RadiusKnown, 140, TURN, game::map::Minefield::MinefieldScanned);

        univ.drawings().addNew(new Drawing(Point(500, 2000), Drawing::MarkerDrawing));

        game::map::Ufo* ufo = univ.ufos().addUfo(1, 2, 3);
        ufo->setPosition(Point(3500, 2000));
        ufo->setRadius(30);

        univ.postprocess(set, set, game::map::Object::ReadOnly, host, config, TURN, shipList, tx, log);

        // Test
        game::map::BoundingBox t;
        t.addUniverse(univ);

        // Minimum X: 500 from drawing, -10
        // Minimum Y: 600 from ion storm, -250 radius
        TS_ASSERT_EQUALS(t.getMinimumCoordinates(), Point( 490,  350));

        // Maximum X: 3500 from Ufo, +30 radius
        // Maximum Y: 3400 from minefield, +140 radius
        TS_ASSERT_EQUALS(t.getMaximumCoordinates(), Point(3531, 3541));
    }

    // East: explosion
    {
        game::map::Universe univ;

        univ.explosions().add(game::map::Explosion(0, Point(700, 3000)));

        univ.postprocess(set, set, game::map::Object::ReadOnly, host, config, TURN, shipList, tx, log);

        // Test
        game::map::BoundingBox t;
        t.addUniverse(univ);

        TS_ASSERT_EQUALS(t.getMinimumCoordinates(), Point( 700, 1000));
        TS_ASSERT_EQUALS(t.getMaximumCoordinates(), Point(3001, 3001));
    }
}

/** Test handling of Ufos with wrap.
    If Ufos are connected, wrap needs to be considered for their counterpart,
    to include the connecting line within the bounding box. */
void
TestGameMapBoundingBox::testAddWrappedUfo()
{
    // Build universe
    game::map::Universe univ;
    univ.config().setConfiguration(game::map::Configuration::Wrapped, Point(2000, 2000), Point(2000, 2000));

    game::map::Ufo* one = univ.ufos().addUfo(1, 2, 3);
    one->setPosition(Point(1100, 2000));
    one->setRadius(30);

    game::map::Ufo* two = univ.ufos().addUfo(10, 2, 3);
    two->setPosition(Point(2900, 2000));
    two->setRadius(40);

    one->connectWith(*two);

    // Test
    game::map::BoundingBox t;
    t.addUniverse(univ);

    // Nearest alias of 2900 is 900, -40 radius
    TS_ASSERT_EQUALS(t.getMinimumCoordinates(), Point( 860, 1000));

    // Nearest alias of 1100 is 3100, +30 radius
    TS_ASSERT_EQUALS(t.getMaximumCoordinates(), Point(3131, 3001));
}

