/**
  *  \file test/game/map/lockertest.cpp
  *  \brief Test for game::map::Locker
  */

#include "game/map/locker.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/map/drawing.hpp"
#include "game/map/minefield.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/test/registrationkey.hpp"

namespace {
    using afl::base::Ref;
    using game::Reference;
    using game::config::HostConfiguration;
    using game::map::Configuration;
    using game::map::Drawing;
    using game::map::Explosion;
    using game::map::Locker;
    using game::map::Minefield;
    using game::map::Planet;
    using game::map::Point;
    using game::map::Ship;
    using game::map::Ufo;
    using game::map::Universe;

    const int ENGINE_TYPE = 9;
    const int HULL_TYPE = 5;

    void createPlanet(Universe& univ, int id, Point pt)
    {
        Planet* p = univ.planets().create(id);
        p->setPosition(pt);

        afl::string::NullTranslator tx;
        afl::sys::Log log;
        p->internalCheck(Configuration(), game::PlayerSet_t(1), 15, tx, log);
    }

    void createShip(Universe& univ, int id, Point pt)
    {
        Ship* p = univ.ships().create(id);
        game::map::ShipData sd;
        sd.x = pt.getX();
        sd.y = pt.getY();
        sd.engineType = ENGINE_TYPE;
        sd.hullType = HULL_TYPE;
        sd.owner = 1;
        p->addCurrentShipData(sd, game::PlayerSet_t(1));
        p->internalCheck(game::PlayerSet_t(1), 15);
    }

    void createUfo(Universe& univ, int id, Point pt)
    {
        Ufo* p = univ.ufos().addUfo(id, 7, 1);
        p->setPosition(pt);
        p->setRadius(5);
    }

    Drawing* createMarker(Point pt, util::Atom_t tag)
    {
        Drawing* p = new Drawing(pt, Drawing::MarkerDrawing);
        p->setTag(tag);
        return p;
    }
}

/** Test addPoint().
    A: call addPoint() several times.
    E: correct point is chosen */
AFL_TEST("game.map.Locker:addPoint", a)
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    t.addPoint(Point(1010, 1000), true);
    t.addPoint(Point(1000, 1010), true);
    t.addPoint(Point(1005, 1005), true);
    t.addPoint(Point( 990, 1000), true);

    a.checkEqual("01. getFoundPoint", t.getFoundPoint(), Point(1005, 1005));
    a.checkEqual("02. getFoundObject", t.getFoundObject(), Reference());
}

/** Test addPoint() with setRangeLimit().
    A: call setRangeLimit(); call addPoint() several times.
    E: correct point is chosen */
AFL_TEST("game.map.Locker:addPoint:setRangeLimit", a)
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    t.setRangeLimit(Point(900, 900), Point(1004, 1004));
    t.addPoint(Point(1010, 1000), true);
    t.addPoint(Point(1000, 1010), true);
    t.addPoint(Point(1005, 1005), true);
    t.addPoint(Point( 990, 1000), true);

    a.checkEqual("01. getFoundPoint", t.getFoundPoint(), Point(990, 1000));
    a.checkEqual("02. getFoundObject", t.getFoundObject(), Reference());
}

/** Test null behaviour.
    A: do not add any points.
    E: original position is returned. */
AFL_TEST("game.map.Locker:null", a)
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    a.checkEqual("01. getFoundPoint", t.getFoundPoint(), Point(1000, 1000));
    a.checkEqual("02. getFoundObject", t.getFoundObject(), Reference());
}

/** Test filtering behaviour.
    A: setMarkedOnly(); add some points.
    E: only marked position is returned. */
AFL_TEST("game.map.Locker:addPoint:setMarkedOnly", a)
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    t.setMarkedOnly(true);
    t.addPoint(Point(1010, 1000), false);
    t.addPoint(Point(1000, 1010), true);
    t.addPoint(Point(1005, 1005), false);
    t.addPoint(Point( 990, 1000), false);

    a.checkEqual("01. getFoundPoint", t.getFoundPoint(), Point(1000, 1010));
    a.checkEqual("02. getFoundObject", t.getFoundObject(), Reference());
}

/** Test locking on planets.
    A: create some planets.
    E: correct position and object returned. */
AFL_TEST("game.map.Locker:addUniverse:planets", a)
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    Universe u;
    createPlanet(u, 50, Point(1020, 1000));
    createPlanet(u, 52, Point(1000, 1019));
    createPlanet(u, 54, Point(1000, 1021));

    t.addUniverse(u, -1, 0);

    a.checkEqual("01. getFoundPoint", t.getFoundPoint(), Point(1000, 1019));
    a.checkEqual("02. getFoundObject", t.getFoundObject(), Reference(Reference::Planet, 52));
}

/** Test locking on ships.
    A: create some ships.
    E: correct position and object returned. */
AFL_TEST("game.map.Locker:addUniverse:ships", a)
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    Universe u;
    createShip(u, 70, Point(1020, 1000));
    createShip(u, 72, Point(1000, 1019));
    createShip(u, 74, Point(1000, 1021));

    t.addUniverse(u, -1, 0);

    a.checkEqual("01. getFoundPoint", t.getFoundPoint(), Point(1000, 1019));
    a.checkEqual("02. getFoundObject", t.getFoundObject(), Reference(Reference::Ship, 72));
}

/** Test locking on Ufos.
    A: create some Ufos.
    E: correct position and object returned. */
AFL_TEST("game.map.Locker:addUniverse:ufos", a)
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    Universe u;
    createUfo(u, 10, Point(1010, 1000));
    createUfo(u, 11, Point( 995, 1005));
    createUfo(u, 12, Point(1001, 1009));

    t.addUniverse(u, -1, 0);

    a.checkEqual("01. getFoundPoint", t.getFoundPoint(), Point(995, 1005));
    a.checkEqual("02. getFoundObject", t.getFoundObject(), Reference(Reference::Ufo, 11));
}

/** Test locking on minefields.
    A: create some minefields.
    E: correct position and object returned. */
AFL_TEST("game.map.Locker:addUniverse:minefields", a)
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    Universe u;
    u.minefields().create(1)->addReport(Point(1010, 1000), 1, Minefield::IsMine, Minefield::UnitsKnown, 50, 1, Minefield::MinefieldScanned);
    u.minefields().create(5)->addReport(Point(1005,  995), 1, Minefield::IsMine, Minefield::UnitsKnown, 50, 1, Minefield::MinefieldScanned);
    u.minefields().create(8)->addReport(Point(1000, 1010), 1, Minefield::IsMine, Minefield::UnitsKnown, 50, 1, Minefield::MinefieldScanned);

    t.addUniverse(u, -1, 0);

    a.checkEqual("01. getFoundPoint", t.getFoundPoint(), Point(1005, 995));
    a.checkEqual("02. getFoundObject", t.getFoundObject(), Reference(Reference::Minefield, 5));
}

/** Test locking on drawings.
    A: create some drawings.
    E: correct position returned. */
AFL_TEST("game.map.Locker:addUniverse:drawings", a)
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    Universe u;
    u.drawings().addNew(new Drawing(Point(990, 1000), Drawing::MarkerDrawing));
    u.drawings().addNew(new Drawing(Point(995, 1000), Drawing::CircleDrawing));    // ignored by Locker
    u.drawings().addNew(new Drawing(Point(1020, 1000), Drawing::MarkerDrawing));

    t.addUniverse(u, -1, 0);

    a.checkEqual("01. getFoundPoint", t.getFoundPoint(), Point(990, 1000));
    a.checkEqual("02. getFoundObject", t.getFoundObject(), Reference());
}

/** Test locking on drawings, with tag filter.
    A: create some drawings with tags.
    E: correct position returned. */
AFL_TEST("game.map.Locker:addUniverse:drawings:with-filter", a)
{
    Configuration fig;
    Universe u;
    u.drawings().addNew(createMarker(Point(990, 1000), 0));
    u.drawings().addNew(createMarker(Point(1020, 1000), 10));

    // Without filter
    {
        Locker t(Point(1000, 1000), fig);
        t.addUniverse(u, -1, 0);
        a.checkEqual("01. getFoundPoint", t.getFoundPoint(), Point(990, 1000));
        a.checkEqual("02. getFoundObject", t.getFoundObject(), Reference());
    }

    // With filter
    {
        Locker t(Point(1000, 1000), fig);
        t.setDrawingTagFilter(10);
        t.addUniverse(u, -1, 0);
        a.checkEqual("11. getFoundPoint", t.getFoundPoint(), Point(1020, 1000));
        a.checkEqual("12. getFoundObject", t.getFoundObject(), Reference());
    }
}

/** Test locking on explosions.
    A: create some explosions.
    E: correct position returned. */
AFL_TEST("game.map.Locker:addUniverse:explosions", a)
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    Universe u;
    u.explosions().add(Explosion(1, Point(990, 1000)));
    u.explosions().add(Explosion(2, Point(995, 1000)));
    u.explosions().add(Explosion(3, Point(1020, 1000)));

    // Explosions are considered drawings
    t.addDrawings(u, 0);

    a.checkEqual("01. getFoundPoint", t.getFoundPoint(), Point(995, 1000));
    a.checkEqual("02. getFoundObject", t.getFoundObject(), Reference());
}

/** Test locking with wrapped map.
    A: create wrapped map. Add some points; closest is across the seam.
    E: point across the seam returned; mapped to be near clicked point. */
AFL_TEST("game.map.Locker:addDrawings:wrap", a)
{
    Configuration fig;
    fig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(2000, 2000));

    Locker t(Point(1010, 1010), fig);

    t.addPoint(Point(1200, 1200), true);
    t.addPoint(Point(2900, 2950), true);

    a.checkEqual("01. getFoundPoint", t.getFoundPoint(), Point(900, 950));
    a.checkEqual("02. getFoundObject", t.getFoundObject(), Reference());
}

/** Test locking with circular wrap.
    A: create circular map. Add some points; closest is across the seam.
    E: point across the seam returned. */
AFL_TEST("game.map.Locker:addDrawings:circular", a)
{
    Configuration fig;
    fig.setConfiguration(Configuration::Circular, Point(2000, 2000), Point(1000, 1000));

    Locker t(Point(2000, 990), fig);

    t.addPoint(Point(2000, 800), true);
    t.addPoint(Point(2000, 2950), true);

    a.checkEqual("01. getFoundPoint", t.getFoundPoint(), Point(2000, 950));
    a.checkEqual("02. getFoundObject", t.getFoundObject(), Reference());

}

/** Test locking at warp well edge.
    A: test some clicked/origin pairs with and without hyperjumping.
    E: verify expected results. */
AFL_TEST("game.map.Locker:findWarpWellEdge", a)
{
    struct TestCase {
        int clickedX, clickedY;
        int originX, originY;
        bool isHyperdriving;
        int shipId;
        int expectX, expectY;
        const char* info;
    };
    const int SHIP_ID = 100;
    static const TestCase TESTS[] = {
        // clicked      origin      HYP       shipId   expect
        // Some standard cases
        { 1000, 1000,   1100, 1000, false,    0,       1003, 1000,   "warp well from east" },
        { 1000, 1000,   1000, 1000, false,    0,       1000, 1000,   "warp well from planet" },
        { 1000, 1000,   1000, 1002, false,    0,       1000, 1000,   "warp well from inside" },
        { 1000, 1000,    500,  500, false,    0,        998,  998,   "warp well from south-east" },

        // With HYP, it can be useful to go a farther distance to be in range.
        { 1000, 1000,   1338, 1000, false,    0,       1003, 1000,   "far normal" },
        { 1000, 1000,   1338, 1000, true,     0,        998, 1000,   "far hyper" },

        // If we cannot ever get into range, don't use any warp wells.
        { 1000, 1000,   1138, 1000, false,    0,       1003, 1000,   "near normal" },
        { 1000, 1000,   1138, 1000, true,     0,       1000, 1000,   "near hyper" },

        // Sometimes it can be required to go farther into a warp well
        { 1000, 1000,   1084, 1013, false,    SHIP_ID, 1002, 1000,   "far warp ship" },
        { 1000, 1000,   1084, 1013, false,    0,       1003, 1000,   "far warp not ship" },
        { 1000, 1000,   1084, 1013, false,    1,       1003, 1000,   "far warp wrong ship" },
    };

    for (size_t i = 0; i < sizeof(TESTS)/sizeof(TESTS[0]); ++i) {
        const TestCase& c = TESTS[i];

        // Environment
        Ref<HostConfiguration> hostConfig = HostConfiguration::create();
        (*hostConfig)[HostConfiguration::RoundGravityWells].set(1);
        game::HostVersion hostVersion(game::HostVersion::PHost, MKVERSION(4,0,0));
        Configuration fig;
        game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);
        game::UnitScoreDefinitionList scoreDefinitions;

        // Minimum ship list
        game::spec::ShipList shipList;
        shipList.engines().create(ENGINE_TYPE)->setMaxEfficientWarp(9);
        shipList.hulls().create(HULL_TYPE)->setMass(100);

        // Universe with a single planet
        Universe u;
        createPlanet(u, 50, Point(1000, 1000));
        createShip(u, SHIP_ID, Point(c.originX, c.originY));

        // Test
        Locker t(Point(c.clickedX, c.clickedY), fig);
        t.addUniverse(u, -1, 0);
        const Point pt = t.findWarpWellEdge(Point(c.originX, c.originY), c.isHyperdriving, u, c.shipId, scoreDefinitions, shipList, *hostConfig, hostVersion, key);

        // Verify
        a(c.info).checkEqual("X", pt.getX(), c.expectX);
        a(c.info).checkEqual("Y", pt.getY(), c.expectY);
    }
}
