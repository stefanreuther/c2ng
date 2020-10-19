/**
  *  \file u/t_game_map_minefieldtype.cpp
  *  \brief Test for game::map::MinefieldType
  */

#include "game/map/minefieldtype.hpp"

#include "t_game_map.hpp"

namespace gp = game::parser;

namespace {
    using game::Id_t;
    using game::map::Minefield;
    using game::map::Point;

    const int TURN = 15;
}

/** Test initial state (empty).
    A: create MinefieldType. Call iteration functions.
    E: must report no content */
void
TestGameMapMinefieldType::testInit()
{
    game::map::MinefieldType testee;
    TS_ASSERT_EQUALS(testee.getNextIndex(0), 0);
    TS_ASSERT_EQUALS(testee.getPreviousIndex(0), 0);
    TS_ASSERT(testee.getObjectByIndex(1) == 0);
    TS_ASSERT(testee.getObjectByIndex(-1) == 0);
}

/** Test iteration.
    A: create MinefieldType and add some minefields. Call iteration functions.
    E: must report correct content */
void
TestGameMapMinefieldType::testIteration()
{
    // Create two minefields
    game::map::MinefieldType testee;
    Minefield* a = testee.create(20);
    a->addReport(Point(1000, 1000), 3, Minefield::IsMine, Minefield::UnitsKnown, 2000, TURN, Minefield::MinefieldScanned);
    Minefield* b = testee.create(30);
    b->addReport(Point(2000, 2000), 4, Minefield::IsMine, Minefield::UnitsKnown, 3000, TURN, Minefield::MinefieldScanned);

    // Iterate
    Id_t ida = testee.findNextIndex(0);
    TS_ASSERT_EQUALS(ida, 20);
    TS_ASSERT_EQUALS(testee.getObjectByIndex(ida), a);
    Id_t idb = testee.findNextIndex(ida);
    TS_ASSERT_EQUALS(idb, 30);
    TS_ASSERT_EQUALS(testee.getObjectByIndex(idb), b);
    TS_ASSERT_EQUALS(testee.findNextIndex(idb), 0);
}

/** Test addMessageInformation() to add minefields, simple case.
    A: create MinefieldType. Call addMessageInformation() with a simple minefield scan.
    E: minefield created and correctly configured */
void
TestGameMapMinefieldType::testAddMessageInformation()
{
    game::map::MinefieldType testee;
    game::HostVersion host;
    game::config::HostConfiguration config;

    {
        gp::MessageInformation info(gp::MessageInformation::Minefield, 30, TURN);
        info.addValue(gp::mi_X, 2222);
        info.addValue(gp::mi_Y, 1555);
        info.addValue(gp::mi_Radius, 40);
        info.addValue(gp::mi_Owner, 10);
        testee.addMessageInformation(info);
        testee.internalCheck(TURN, host, config);
    }

    Minefield* mf = testee.get(30);
    TS_ASSERT(mf);

    game::map::Point pt;
    TS_ASSERT_EQUALS(mf->getPosition(pt), true);
    TS_ASSERT_EQUALS(pt.getX(), 2222);
    TS_ASSERT_EQUALS(pt.getY(), 1555);

    int n;
    TS_ASSERT_EQUALS(mf->getRadius(n), true);
    TS_ASSERT_EQUALS(n, 40);

    TS_ASSERT_EQUALS(mf->getOwner(n), true);
    TS_ASSERT_EQUALS(n, 10);
}

/** Test addMessageInformation() to add minefields, complex case.
    A: create MinefieldType. Call addMessageInformation() with a complex minefield scan (all fields set).
    E: minefield created and correctly configured */
void
TestGameMapMinefieldType::testAddMessageInformationFull()
{
    game::map::MinefieldType testee;
    game::HostVersion host;
    game::config::HostConfiguration config;

    {
        gp::MessageInformation info(gp::MessageInformation::Minefield, 30, TURN);
        info.addValue(gp::mi_X, 1666);
        info.addValue(gp::mi_Y, 1777);;
        info.addValue(gp::mi_Radius, 40);
        info.addValue(gp::mi_Owner, 10);
        info.addValue(gp::mi_MineUnits, 1620);
        info.addValue(gp::mi_Type, 1);
        info.addValue(gp::mi_MineScanReason, 2);
        testee.addMessageInformation(info);
        testee.internalCheck(TURN, host, config);
    }

    Minefield* mf = testee.get(30);
    TS_ASSERT(mf);

    game::map::Point pt;
    TS_ASSERT_EQUALS(mf->getPosition(pt), true);
    TS_ASSERT_EQUALS(pt.getX(), 1666);
    TS_ASSERT_EQUALS(pt.getY(), 1777);

    int n;
    TS_ASSERT_EQUALS(mf->getRadius(n), true);
    TS_ASSERT_EQUALS(n, 40);

    TS_ASSERT_EQUALS(mf->getOwner(n), true);
    TS_ASSERT_EQUALS(n, 10);

    TS_ASSERT_EQUALS(mf->getUnits(), 1620);
    TS_ASSERT_EQUALS(mf->getReason(), Minefield::MinefieldSwept);
    TS_ASSERT_EQUALS(mf->isWeb(), true);
}

/** Test addMessageInformation() to add minefields, simple case.
    A: create MinefieldType. Create a minefield. Call addMessageInformation() with a radius update.
    E: radius correctly updated */
void
TestGameMapMinefieldType::testAddMessageInformationMinUpdate()
{
    game::map::MinefieldType testee;
    game::HostVersion host;
    game::config::HostConfiguration config;

    testee.create(333)->addReport(Point(1444, 1555), 4, Minefield::IsMine, Minefield::UnitsKnown, 3000, TURN, Minefield::MinefieldScanned);

    {
        gp::MessageInformation info(gp::MessageInformation::Minefield, 333, TURN);
        info.addValue(gp::mi_Radius, 40);
        testee.addMessageInformation(info);
        testee.internalCheck(TURN, host, config);
    }

    Minefield* mf = testee.get(333);
    TS_ASSERT(mf);

    game::map::Point pt;
    TS_ASSERT_EQUALS(mf->getPosition(pt), true);
    TS_ASSERT_EQUALS(pt.getX(), 1444);
    TS_ASSERT_EQUALS(pt.getY(), 1555);

    int n;
    TS_ASSERT_EQUALS(mf->getRadius(n), true);
    TS_ASSERT_EQUALS(n, 40);

    TS_ASSERT_EQUALS(mf->getOwner(n), true);
    TS_ASSERT_EQUALS(n, 4);

    TS_ASSERT_EQUALS(mf->getUnits(), 1600);
    TS_ASSERT_EQUALS(mf->getReason(), Minefield::MinefieldScanned);
    TS_ASSERT_EQUALS(mf->isWeb(), false);
}

/** Test addMessageInformation() to add minefields, minimal information, failure.
    A: create MinefieldType. Do NOT create a minefield. Call addMessageInformation() with a radius update.
    E: no minefield created */
void
TestGameMapMinefieldType::testAddMessageInformationMinFail()
{
    game::map::MinefieldType testee;
    game::HostVersion host;
    game::config::HostConfiguration config;

    {
        gp::MessageInformation info(gp::MessageInformation::Minefield, 333, TURN);
        info.addValue(gp::mi_Radius, 40);
        testee.addMessageInformation(info);
        testee.internalCheck(TURN, host, config);
    }

    Minefield* mf = testee.get(333);
    TS_ASSERT(mf == 0);
}

/** Test erase().
    A: create MinefieldType. Add minefields. Erase one.
    E: erased minefield reports !isValid and is not part of iteration. */
void
TestGameMapMinefieldType::testErase()
{
    // Create two minefields
    game::map::MinefieldType testee;
    Minefield* a = testee.create(20);
    a->addReport(Point(1000, 1000), 3, Minefield::IsMine, Minefield::UnitsKnown, 2000, TURN, Minefield::MinefieldScanned);
    Minefield* b = testee.create(30);
    b->addReport(Point(2000, 2000), 4, Minefield::IsMine, Minefield::UnitsKnown, 3000, TURN, Minefield::MinefieldScanned);

    // Erase some
    testee.erase(444);
    testee.erase(20);

    // Verify
    TS_ASSERT(!a->isValid());
    TS_ASSERT(b->isValid());

    // Verify iteration
    TS_ASSERT_EQUALS(testee.findNextIndex(0), 30);
    TS_ASSERT_EQUALS(testee.findNextIndex(30), 0);
}

/** Test handling of setAllMinefieldsKnown().
    A: create MinefieldType. Add minefields of different races and turns. Declare one race as all-minefields-known. Call internalCheck().
    E: old minefields of declared race are marked deleted */
void
TestGameMapMinefieldType::testAllMinefieldsKnown()
{
    const int THEM = 3;
    const int ME = 4;

    game::map::MinefieldType testee;
    game::HostVersion host;
    game::config::HostConfiguration config;

    Minefield* theirOld = testee.create(101);
    theirOld->addReport(Point(1000, 1000), THEM, Minefield::IsMine, Minefield::UnitsKnown, 2000, TURN-1, Minefield::MinefieldScanned);

    Minefield* theirNew = testee.create(102);
    theirNew->addReport(Point(1000, 1000), THEM, Minefield::IsMine, Minefield::UnitsKnown, 3000, TURN, Minefield::MinefieldScanned);

    Minefield* myOld = testee.create(201);
    myOld->addReport(Point(1000, 1000), ME, Minefield::IsMine, Minefield::UnitsKnown, 2000, TURN-1, Minefield::MinefieldScanned);

    Minefield* myNew = testee.create(202);
    myNew->addReport(Point(1000, 1000), ME, Minefield::IsMine, Minefield::UnitsKnown, 3000, TURN, Minefield::MinefieldScanned);

    testee.setAllMinefieldsKnown(ME);
    testee.internalCheck(TURN, host, config);

    // Verify
    TS_ASSERT(theirOld->isValid());
    TS_ASSERT(theirNew->isValid());
    TS_ASSERT(!myOld->isValid());
    TS_ASSERT(myNew->isValid());
}

