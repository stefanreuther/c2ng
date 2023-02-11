/**
  *  \file u/t_game_map_shiphistorydata.cpp
  *  \brief Test for game::map::ShipHistoryData
  */

#include "game/map/shiphistorydata.hpp"

#include "t_game_map.hpp"

using game::map::ShipHistoryData;

/** Test initialisation.
    Create an empty object, add some data; verify content. */
void
TestGameMapShipHistoryData::testInit()
{
    ShipHistoryData t;

    ShipHistoryData::Track* t50 = adjustShipHistory(t, 50);
    TS_ASSERT(t50 != 0);
    t50->x = 1000;
    t50->y = 1100;

    ShipHistoryData::Track* t52 = adjustShipHistory(t, 52);
    TS_ASSERT(t52 != 0);
    t52->x = 1200;
    t52->y = 1300;

    // Verify
    TS_ASSERT(getShipHistory(t, 50) != 0);
    TS_ASSERT(getShipHistory(t, 51) != 0);    // between two created items
    TS_ASSERT(getShipHistory(t, 52) != 0);

    TS_ASSERT(getShipHistory(t, 53) == 0);    // too new
    TS_ASSERT(getShipHistory(t, 1) == 0);     // too old

    TS_ASSERT_EQUALS(getShipHistory(t, 50)->x.orElse(-1), 1000);
    TS_ASSERT_EQUALS(getShipHistory(t, 51)->x.orElse(-1), -1);
    TS_ASSERT_EQUALS(getShipHistory(t, 52)->x.orElse(-1), 1200);
}

/** Test clearing. */
void
TestGameMapShipHistoryData::testClear()
{
    ShipHistoryData t;

    ShipHistoryData::Track* t50 = adjustShipHistory(t, 50);
    TS_ASSERT(t50 != 0);
    t50->x = 1000;
    t50->y = 1100;

    // Try to clear too-new entry
    clearShipHistory(t, 100);

    // Existing entry remains visible
    TS_ASSERT(getShipHistory(t, 50) != 0);
    TS_ASSERT_EQUALS(getShipHistory(t, 50)->x.orElse(-1), 1000);

    // Try to clear existing entry
    clearShipHistory(t, 50);
    TS_ASSERT(getShipHistory(t, 50) != 0);
    TS_ASSERT_EQUALS(getShipHistory(t, 50)->x.orElse(-1), -1);

    // Clear everything
    clearShipHistory(t);
    TS_ASSERT(getShipHistory(t, 50) == 0);
}

