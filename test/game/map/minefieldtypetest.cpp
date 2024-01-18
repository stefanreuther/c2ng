/**
  *  \file test/game/map/minefieldtypetest.cpp
  *  \brief Test for game::map::MinefieldType
  */

#include "game/map/minefieldtype.hpp"
#include "afl/test/testrunner.hpp"

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
AFL_TEST("game.map.MinefieldType:init", a)
{
    game::map::MinefieldType testee;
    a.checkEqual("01. getNextIndex", testee.getNextIndex(0), 0);
    a.checkEqual("02. getPreviousIndex", testee.getPreviousIndex(0), 0);
    a.checkNull("03. getObjectByIndex", testee.getObjectByIndex(1));
    a.checkNull("04. getObjectByIndex", testee.getObjectByIndex(-1));
}

/** Test iteration.
    A: create MinefieldType and add some minefields. Call iteration functions.
    E: must report correct content */
AFL_TEST("game.map.MinefieldType:iteration", a)
{
    // Create two minefields
    game::map::MinefieldType testee;
    Minefield* ma = testee.create(20);
    ma->addReport(Point(1000, 1000), 3, Minefield::IsMine, Minefield::UnitsKnown, 2000, TURN, Minefield::MinefieldScanned);
    Minefield* mb = testee.create(30);
    mb->addReport(Point(2000, 2000), 4, Minefield::IsMine, Minefield::UnitsKnown, 3000, TURN, Minefield::MinefieldScanned);

    // Iterate
    Id_t ida = testee.findNextIndex(0);
    a.checkEqual("01. findNextIndex", ida, 20);
    a.checkEqual("02. getObjectByIndex", testee.getObjectByIndex(ida), ma);
    Id_t idb = testee.findNextIndex(ida);
    a.checkEqual("03. findNextIndex", idb, 30);
    a.checkEqual("04. getObjectByIndex", testee.getObjectByIndex(idb), mb);
    a.checkEqual("05. findNextIndex", testee.findNextIndex(idb), 0);
}

/** Test addMessageInformation() to add minefields, simple case.
    A: create MinefieldType. Call addMessageInformation() with a simple minefield scan.
    E: minefield created and correctly configured */
AFL_TEST("game.map.MinefieldType:addMessageInformation:simple", a)
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
    a.check("01. get", mf);

    game::map::Point pt;
    a.checkEqual("11. getPosition", mf->getPosition().get(pt), true);
    a.checkEqual("12. X", pt.getX(), 2222);
    a.checkEqual("13. Y", pt.getY(), 1555);

    int n;
    a.checkEqual("21. getRadius", mf->getRadius().get(n), true);
    a.checkEqual("22. radius", n, 40);

    a.checkEqual("31. getOwner", mf->getOwner().get(n), true);
    a.checkEqual("32. owner", n, 10);
}

/** Test addMessageInformation() to add minefields, complex case.
    A: create MinefieldType. Call addMessageInformation() with a complex minefield scan (all fields set).
    E: minefield created and correctly configured */
AFL_TEST("game.map.MinefieldType:addMessageInformation:full", a)
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
    a.check("01", mf);

    game::map::Point pt;
    a.checkEqual("11. getPosition", mf->getPosition().get(pt), true);
    a.checkEqual("12. X", pt.getX(), 1666);
    a.checkEqual("13. Y", pt.getY(), 1777);

    int n;
    a.checkEqual("21. getRadius", mf->getRadius().get(n), true);
    a.checkEqual("22. radius", n, 40);

    a.checkEqual("31. getOwner", mf->getOwner().get(n), true);
    a.checkEqual("32. owner", n, 10);

    a.checkEqual("41. getUnits", mf->getUnits(), 1620);
    a.checkEqual("42. getRadius", mf->getReason(), Minefield::MinefieldSwept);
    a.checkEqual("43. isWeb", mf->isWeb(), true);
}

/** Test addMessageInformation() to add minefields, simple case.
    A: create MinefieldType. Create a minefield. Call addMessageInformation() with a radius update.
    E: radius correctly updated */
AFL_TEST("game.map.MinefieldType:addMessageInformation:min-update", a)
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
    a.check("01", mf);

    game::map::Point pt;
    a.checkEqual("11. getPosition", mf->getPosition().get(pt), true);
    a.checkEqual("12. X", pt.getX(), 1444);
    a.checkEqual("13. Y", pt.getY(), 1555);

    int n;
    a.checkEqual("21. getRadius", mf->getRadius().get(n), true);
    a.checkEqual("22. radius", n, 40);

    a.checkEqual("31. getOwner", mf->getOwner().get(n), true);
    a.checkEqual("32. owner", n, 4);

    a.checkEqual("41. getUnits", mf->getUnits(), 1600);
    a.checkEqual("42. getRadius", mf->getReason(), Minefield::MinefieldScanned);
    a.checkEqual("43. isWeb", mf->isWeb(), false);
}

/** Test addMessageInformation() to add minefields, minimal information, failure.
    A: create MinefieldType. Do NOT create a minefield. Call addMessageInformation() with a radius update.
    E: no minefield created */
AFL_TEST("game.map.MinefieldType:addMessageInformation:min-fail", a)
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
    a.checkNull("01. get", mf);
}

/** Test erase().
    A: create MinefieldType. Add minefields. Erase one.
    E: erased minefield reports !isValid and is not part of iteration. */
AFL_TEST("game.map.MinefieldType:erase", a)
{
    // Create two minefields
    game::map::MinefieldType testee;
    Minefield* ma = testee.create(20);
    ma->addReport(Point(1000, 1000), 3, Minefield::IsMine, Minefield::UnitsKnown, 2000, TURN, Minefield::MinefieldScanned);
    Minefield* mb = testee.create(30);
    mb->addReport(Point(2000, 2000), 4, Minefield::IsMine, Minefield::UnitsKnown, 3000, TURN, Minefield::MinefieldScanned);

    // Erase some
    testee.erase(444);
    testee.erase(20);

    // Verify
    a.check("01. isValid", !ma->isValid());
    a.check("02. isValid", mb->isValid());

    // Verify iteration
    a.checkEqual("11. findNextIndex", testee.findNextIndex(0), 30);
    a.checkEqual("12. findNextIndex", testee.findNextIndex(30), 0);
}

/** Test handling of setAllMinefieldsKnown().
    A: create MinefieldType. Add minefields of different races and turns. Declare one race as all-minefields-known. Call internalCheck().
    E: old minefields of declared race are marked deleted */
AFL_TEST("game.map.MinefieldType:setAllMinefieldsKnown", a)
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
    a.check("01. isValid", theirOld->isValid());
    a.check("02. isValid", theirNew->isValid());
    a.check("03. isValid", !myOld->isValid());
    a.check("04. isValid", myNew->isValid());
}
