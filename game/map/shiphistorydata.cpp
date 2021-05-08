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

game::map::ShipHistoryData::Track*
game::map::adjustShipHistory(ShipHistoryData& d, int turn)
{
    // ex GShip::adjustShipTrack
    if (turn > d.trackTurn) {
        int adjust = turn - d.trackTurn;
        for (int i = NUM_SHIP_TRACK_ENTRIES-1; i >= 0; --i) {
            d.track[i] = (i >= adjust
                          ? d.track[i - adjust]
                          : ShipHistoryData::Track());
        }
        d.trackTurn = turn;
    }

    size_t offset = d.trackTurn - turn;
    if (offset < NUM_SHIP_TRACK_ENTRIES) {
        return &d.track[offset];
    } else {
        return 0;
    }
}

game::map::ShipHistoryData::Track*
game::map::getShipHistory(ShipHistoryData& d, int turn)
{
    if (turn <= d.trackTurn) {
        size_t offset = d.trackTurn - turn;
        if (offset < NUM_SHIP_TRACK_ENTRIES) {
            return &d.track[offset];
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}
