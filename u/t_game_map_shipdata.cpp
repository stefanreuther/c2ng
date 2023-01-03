/**
  *  \file u/t_game_map_shipdata.cpp
  *  \brief Test for game::map::ShipData
  */

#include "game/map/shipdata.hpp"

#include "t_game_map.hpp"

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
void
TestGameMapShipData::testGetShipMassEmpty()
{
    ShipList list;
    ShipData testee;
    TS_ASSERT(!getShipMass(testee, list).isValid());
}

/** Test getShipMass, freighter.
    A: getShipMass() on freighter, hull is known.
    E: returns accepted data. */
void
TestGameMapShipData::testGetShipMassFreighter()
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

    TS_ASSERT_EQUALS(getShipMass(testee, list).orElse(-1), 415);
}

/** Test getShipMass, capital ship.
    A: getShipMass() on capital ship, all components known.
    E: returns accepted data. */
void
TestGameMapShipData::testGetShipMassCapital()
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

    TS_ASSERT_EQUALS(getShipMass(testee, list).orElse(-1), 680);
}

/** Test getShipMass, unknown hull.
    A: getShipMass() on ship whose hull is not defined.
    E: returns unknown. */
void
TestGameMapShipData::testGetShipMassNoHull()
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

    TS_ASSERT(!getShipMass(testee, list).isValid());
}

/** Test getShipMass, unknown beam.
    A: getShipMass() on ship whose beam is not defined.
    E: returns unknown. */
void
TestGameMapShipData::testGetShipMassNoBeam()
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

    TS_ASSERT(!getShipMass(testee, list).isValid());
}

/** Test getShipMass, unknown launcher.
    A: getShipMass() on ship whose launcher is not defined.
    E: returns unknown. */
void
TestGameMapShipData::testGetShipMassNoLauncher()
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

    TS_ASSERT(!getShipMass(testee, list).isValid());
}

/** Test isTransferActive(), empty.
    A: call isTransferActive on entirely empty Transfer
    E: false */
void
TestGameMapShipData::testIsTransferActiveEmpty()
{
    ShipData::Transfer testee;
    TS_ASSERT_EQUALS(isTransferActive(testee), false);
}

/** Test isTransferActive(), full.
    A: call isTransferActive on fully populated Transfer
    E: true */
void
TestGameMapShipData::testIsTransferActiveFull()
{
    ShipData::Transfer testee;
    testee.targetId   = 1;
    testee.neutronium = 2;
    testee.duranium   = 3;
    testee.tritanium  = 4;
    testee.molybdenum = 5;
    testee.supplies   = 6;
    testee.colonists  = 7;
    TS_ASSERT_EQUALS(isTransferActive(testee), true);
}

/** Test isTransferActive(), partial.
    A: call isTransferActive on sparsely populated Transfer. In particular, no Id (this is the Jettison case).
    E: true */
void
TestGameMapShipData::testIsTransferActivePart()
{
    ShipData::Transfer testee;
    testee.targetId   = 0;
    testee.neutronium = 2;
    testee.duranium   = 0;
    testee.tritanium  = 0;
    testee.molybdenum = 0;
    testee.supplies   = 0;
    testee.colonists  = 0;
    TS_ASSERT_EQUALS(isTransferActive(testee), true);
}

