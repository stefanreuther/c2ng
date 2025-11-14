/**
  *  \file test/game/map/boundingboxtest.cpp
  *  \brief Test for game::map::BoundingBox
  */

#include "game/map/boundingbox.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/map/drawing.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"

using game::map::Drawing;
using game::map::Point;

/** Test default initialisation. */
AFL_TEST("game.map.BoundingBox:init", a)
{
    game::map::BoundingBox t;
    a.checkEqual("equal borders", t.getMinimumCoordinates(), t.getMaximumCoordinates());
}

/** Test addPoint(). */
AFL_TEST("game.map.BoundingBox:addPoint", a)
{
    game::map::BoundingBox t;
    t.addPoint(Point(1000, 2000));
    a.checkEqual("01. min", t.getMinimumCoordinates(), Point(1000, 2000));
    a.checkEqual("02. max", t.getMaximumCoordinates(), Point(1001, 2001));

    t.addPoint(Point(1500, 1400));
    a.checkEqual("11. min", t.getMinimumCoordinates(), Point(1000, 1400));
    a.checkEqual("12. max", t.getMaximumCoordinates(), Point(1501, 2001));
}

/** Test addCircle(). */
AFL_TEST("game.map.BoundingBox:addCircle", a)
{
    game::map::BoundingBox t;
    t.addCircle(Point(1200, 1300), 30);

    a.checkEqual("min", t.getMinimumCoordinates(), Point(1170, 1270));
    a.checkEqual("max", t.getMaximumCoordinates(), Point(1231, 1331));
}

/*
 *  addDrawing() / all drawing types
 */

// Line
AFL_TEST("game.map.BoundingBox:addDrawing:LineDrawing", a)
{
    game::map::BoundingBox t;
    Drawing d(Point(1000, 2000), Drawing::LineDrawing);
    d.setPos2(Point(1200, 1400));
    t.addDrawing(d);

    a.checkEqual("min", t.getMinimumCoordinates(), Point(1000, 1400));
    a.checkEqual("max", t.getMaximumCoordinates(), Point(1201, 2001));
}

// Rectangle
AFL_TEST("game.map.BoundingBox:addDrawing:RectangleDrawing", a)
{
    game::map::BoundingBox t;
    Drawing d(Point(1200, 1300), Drawing::RectangleDrawing);
    d.setPos2(Point(1400, 1500));
    t.addDrawing(d);

    a.checkEqual("min", t.getMinimumCoordinates(), Point(1200, 1300));
    a.checkEqual("max", t.getMaximumCoordinates(), Point(1401, 1501));
}

// Circle
AFL_TEST("game.map.BoundingBox:addDrawing:CircleDrawing", a)
{
    game::map::BoundingBox t;
    Drawing d(Point(1100, 1500), Drawing::CircleDrawing);
    d.setCircleRadius(25);
    t.addDrawing(d);

    a.checkEqual("min", t.getMinimumCoordinates(), Point(1075, 1475));
    a.checkEqual("max", t.getMaximumCoordinates(), Point(1126, 1526));
}

// Marker
AFL_TEST("game.map.BoundingBox:addDrawing:MarkerDrawing", a)
{
    game::map::BoundingBox t;
    Drawing d(Point(1500, 1600), Drawing::MarkerDrawing);
    t.addDrawing(d);

    a.checkEqual("min", t.getMinimumCoordinates(), Point(1490, 1590));
    a.checkEqual("max", t.getMaximumCoordinates(), Point(1511, 1611));
}


/** Test addUniverse().
    Test multiple universes, and prove for each that all objects are considered. */
AFL_TEST("game.map.BoundingBox:addUniverse", a)
{
    game::PlayerSet_t set(1);
    game::HostVersion host(game::HostVersion::Host, MKVERSION(3,22,44));
    afl::base::Ref<game::config::HostConfiguration> config = game::config::HostConfiguration::create();
    game::map::Configuration mapConfig;
    game::spec::ShipList shipList;
    shipList.hulls().create(1);
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    const int TURN = 10;

    // East: planet, west: ship. North/south borders determined by map config
    {
        game::map::Universe univ;
        a.checkEqual("01. map min", mapConfig.getMinimumCoordinates(), Point(1000, 1000));
        a.checkEqual("02. map max", mapConfig.getMaximumCoordinates(), Point(3000, 3000));

        game::map::Planet* p = univ.planets().create(10);
        p->setPosition(Point(500, 2000));
        p->setOwner(0);

        game::map::Ship* sh = univ.ships().create(20);
        sh->addShipXYData(Point(3400, 2000), 2, 500, set);

        univ.postprocess(set, set, game::map::Object::ReadOnly, mapConfig, host, *config, TURN, shipList, tx, log);

        // Test
        game::map::BoundingBox t;
        t.addUniverse(univ, mapConfig);

        a.checkEqual("11. min", t.getMinimumCoordinates(), Point( 500, 1000));
        a.checkEqual("12. max", t.getMaximumCoordinates(), Point(3401, 3001));
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

        univ.postprocess(set, set, game::map::Object::ReadOnly, mapConfig, host, *config, TURN, shipList, tx, log);

        // Test
        game::map::BoundingBox t;
        t.addUniverse(univ, mapConfig);

        // Minimum X: 500 from drawing, -10
        // Minimum Y: 600 from ion storm, -250 radius
        a.checkEqual("21. min", t.getMinimumCoordinates(), Point( 490,  350));

        // Maximum X: 3500 from Ufo, +30 radius
        // Maximum Y: 3400 from minefield, +140 radius
        a.checkEqual("31. max", t.getMaximumCoordinates(), Point(3531, 3541));
    }

    // East: explosion
    {
        game::map::Universe univ;

        univ.explosions().add(game::map::Explosion(0, Point(700, 3000)));

        univ.postprocess(set, set, game::map::Object::ReadOnly, mapConfig, host, *config, TURN, shipList, tx, log);

        // Test
        game::map::BoundingBox t;
        t.addUniverse(univ, mapConfig);

        a.checkEqual("41. min", t.getMinimumCoordinates(), Point( 700, 1000));
        a.checkEqual("42. max", t.getMaximumCoordinates(), Point(3001, 3001));
    }
}

/** Test handling of Ufos with wrap.
    If Ufos are connected, wrap needs to be considered for their counterpart,
    to include the connecting line within the bounding box. */
AFL_TEST("game.map.BoundingBox:addUniverse:ufo-in-wrapped-universe", a)
{
    // Build universe
    game::map::Universe univ;
    game::map::Configuration mapConfig;
    mapConfig.setConfiguration(game::map::Configuration::Wrapped, Point(2000, 2000), Point(2000, 2000));

    game::map::Ufo* one = univ.ufos().addUfo(1, 2, 3);
    one->setPosition(Point(1100, 2000));
    one->setRadius(30);

    game::map::Ufo* two = univ.ufos().addUfo(10, 2, 3);
    two->setPosition(Point(2900, 2000));
    two->setRadius(40);

    one->connectWith(*two);

    // Test
    game::map::BoundingBox t;
    t.addUniverse(univ, mapConfig);

    // Nearest alias of 2900 is 900, -40 radius
    a.checkEqual("01. min", t.getMinimumCoordinates(), Point( 860, 1000));

    // Nearest alias of 1100 is 3100, +30 radius
    a.checkEqual("11. max", t.getMaximumCoordinates(), Point(3131, 3001));
}
