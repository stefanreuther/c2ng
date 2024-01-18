/**
  *  \file test/game/map/shipdatatest.cpp
  *  \brief Test for game::map::ShipData
  */

#include "game/map/shipdata.hpp"
#include "afl/test/testrunner.hpp"

using game::spec::ShipList;
using game::map::ShipData;

namespace {
    void setCargo(ShipData& t)
    {
        t.ammo = 0;
        t.neutronium = 10;
        t.tritanium = 20;
        t.duranium = 30;
        t.molybdenum = 40;
        t.colonists = 50;
        t.supplies = 60;
        t.money = 70;
        t.ammo = 5;
        // total: 215
    }
}

/** Test getShipMass, empty.
    A: getShipMass() on uninitialized ShipData.
    E: returns unknown */
AFL_TEST("game.map.ShipData:getShipMass:empty", a)
{
    ShipList list;
    ShipData testee;
    a.check("", !getShipMass(testee, list).isValid());
}

/** Test getShipMass, freighter.
    A: getShipMass() on freighter, hull is known.
    E: returns accepted data. */
AFL_TEST("game.map.ShipData:getShipMass:freighter", a)
{
    ShipList list;
    list.hulls().create(16)->setMass(200);

    ShipData testee;
    testee.hullType = 16;

    setCargo(testee);
    testee.numLaunchers = 0;
    testee.torpedoType = 0;
    testee.numBeams = 0;
    testee.beamType = 0;

    a.checkEqual("", getShipMass(testee, list).orElse(-1), 415);
}

/** Test getShipMass, capital ship.
    A: getShipMass() on capital ship, all components known.
    E: returns accepted data. */
AFL_TEST("game.map.ShipData:getShipMass:capital", a)
{
    ShipList list;
    list.hulls().create(20)->setMass(400);
    list.launchers().create(3)->setMass(5);
    list.beams().create(4)->setMass(6);

    ShipData testee;
    testee.hullType = 20;

    setCargo(testee);
    testee.numLaunchers = 7;     /* 7*5 = 35 */
    testee.torpedoType = 3;
    testee.numBeams = 5;         /* 5*6 = 30 */
    testee.beamType = 4;

    a.checkEqual("", getShipMass(testee, list).orElse(-1), 680);
}

/** Test getShipMass, unknown hull.
    A: getShipMass() on ship whose hull is not defined.
    E: returns unknown. */
AFL_TEST("game.map.ShipData:getShipMass:unknown-hull", a)
{
    ShipList list;
    // No hull
    list.launchers().create(3)->setMass(5);
    list.beams().create(4)->setMass(6);

    ShipData testee;
    testee.hullType = 20;

    setCargo(testee);
    testee.numLaunchers = 7;
    testee.torpedoType = 3;
    testee.numBeams = 5;
    testee.beamType = 4;

    a.check("", !getShipMass(testee, list).isValid());
}

/** Test getShipMass, unknown beam.
    A: getShipMass() on ship whose beam is not defined.
    E: returns unknown. */
AFL_TEST("game.map.ShipData:getShipMass:unknown-beam", a)
{
    ShipList list;
    list.hulls().create(20)->setMass(400);
    list.launchers().create(3)->setMass(5);
    // No beam

    ShipData testee;
    testee.hullType = 20;

    setCargo(testee);
    testee.numLaunchers = 7;
    testee.torpedoType = 3;
    testee.numBeams = 5;
    testee.beamType = 4;

    a.check("", !getShipMass(testee, list).isValid());
}

/** Test getShipMass, unknown launcher.
    A: getShipMass() on ship whose launcher is not defined.
    E: returns unknown. */
AFL_TEST("game.map.ShipData:getShipMass:unknown-launcher", a)
{
    ShipList list;
    list.hulls().create(20)->setMass(400);
    // No launcher
    list.beams().create(4)->setMass(6);

    ShipData testee;
    testee.hullType = 20;

    setCargo(testee);
    testee.numLaunchers = 7;
    testee.torpedoType = 3;
    testee.numBeams = 5;
    testee.beamType = 4;

    a.check("", !getShipMass(testee, list).isValid());
}

/** Test isTransferActive(), empty.
    A: call isTransferActive on entirely empty Transfer
    E: false */
AFL_TEST("game.map.ShipData:isTransferActive:empty", a)
{
    ShipData::Transfer testee;
    a.checkEqual("", isTransferActive(testee), false);
}

/** Test isTransferActive(), full.
    A: call isTransferActive on fully populated Transfer
    E: true */
AFL_TEST("game.map.ShipData:isTransferActive:full", a)
{
    ShipData::Transfer testee;
    testee.targetId   = 1;
    testee.neutronium = 2;
    testee.duranium   = 3;
    testee.tritanium  = 4;
    testee.molybdenum = 5;
    testee.supplies   = 6;
    testee.colonists  = 7;
    a.checkEqual("", isTransferActive(testee), true);
}

/** Test isTransferActive(), partial.
    A: call isTransferActive on sparsely populated Transfer. In particular, no Id (this is the Jettison case).
    E: true */
AFL_TEST("game.map.ShipData:isTransferActive:partial", a)
{
    ShipData::Transfer testee;
    testee.targetId   = 0;
    testee.neutronium = 2;
    testee.duranium   = 0;
    testee.tritanium  = 0;
    testee.molybdenum = 0;
    testee.supplies   = 0;
    testee.colonists  = 0;
    a.checkEqual("", isTransferActive(testee), true);
}
