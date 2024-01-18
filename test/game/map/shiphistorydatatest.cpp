/**
  *  \file test/game/map/shiphistorydatatest.cpp
  *  \brief Test for game::map::ShipHistoryData
  */

#include "game/map/shiphistorydata.hpp"
#include "afl/test/testrunner.hpp"

using game::map::ShipHistoryData;

/** Test initialisation.
    Create an empty object, add some data; verify content. */
AFL_TEST("game.map.ShipHistoryData:basic", a)
{
    ShipHistoryData t;

    ShipHistoryData::Track* t50 = adjustShipHistory(t, 50);
    a.checkNonNull("01. adjustShipHistory", t50);
    t50->x = 1000;
    t50->y = 1100;

    ShipHistoryData::Track* t52 = adjustShipHistory(t, 52);
    a.checkNonNull("11. adjustShipHistory", t52);
    t52->x = 1200;
    t52->y = 1300;

    // Verify
    a.checkNonNull("21. getShipHistory", getShipHistory(t, 50));
    a.checkNonNull("22. getShipHistory", getShipHistory(t, 51));    // between two created items
    a.checkNonNull("23. getShipHistory", getShipHistory(t, 52));

    a.checkNull("31. getShipHistory", getShipHistory(t, 53));    // too new
    a.checkNull("32. getShipHistory", getShipHistory(t, 1));     // too old

    a.checkEqual("41. getShipHistory", getShipHistory(t, 50)->x.orElse(-1), 1000);
    a.checkEqual("42. getShipHistory", getShipHistory(t, 51)->x.orElse(-1), -1);
    a.checkEqual("43. getShipHistory", getShipHistory(t, 52)->x.orElse(-1), 1200);
}

/** Test clearing. */
AFL_TEST("game.map.ShipHistoryData:clear", a)
{
    ShipHistoryData t;

    ShipHistoryData::Track* t50 = adjustShipHistory(t, 50);
    a.checkNonNull("01. adjustShipHistory", t50);
    t50->x = 1000;
    t50->y = 1100;

    // Try to clear too-new entry
    clearShipHistory(t, 100);

    // Existing entry remains visible
    a.checkNonNull("11. getShipHistory", getShipHistory(t, 50));
    a.checkEqual("12. getShipHistory", getShipHistory(t, 50)->x.orElse(-1), 1000);

    // Try to clear existing entry
    clearShipHistory(t, 50);
    a.checkNonNull("21. getShipHistory", getShipHistory(t, 50));
    a.checkEqual("22. getShipHistory", getShipHistory(t, 50)->x.orElse(-1), -1);

    // Clear everything
    clearShipHistory(t);
    a.checkNull("31. getShipHistory", getShipHistory(t, 50));
}
