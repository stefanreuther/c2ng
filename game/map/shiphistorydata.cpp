/**
  *  \file game/map/shiphistorydata.cpp
  */

#include "game/map/shiphistorydata.hpp"

void
game::map::clearShipHistory(ShipHistoryData& d)
{
    d.trackTurn = 0;
    for (size_t i = 0; i < NUM_SHIP_TRACK_ENTRIES; ++i) {
        d.track[i] = ShipHistoryData::Track();
    }
}
