/**
  *  \file game/map/shiphistorydata.hpp
  */
#ifndef C2NG_GAME_MAP_SHIPHISTORYDATA_HPP
#define C2NG_GAME_MAP_SHIPHISTORYDATA_HPP

#include "game/map/point.hpp"
#include "game/types.hpp"

namespace game { namespace map {

    const size_t NUM_SHIP_TRACK_ENTRIES = 10;

    struct ShipHistoryData {
        struct Track {
            IntegerProperty_t x;
            IntegerProperty_t y;
            IntegerProperty_t speed;
            IntegerProperty_t heading;
            IntegerProperty_t mass;
        };

        int trackTurn;
        Track track[NUM_SHIP_TRACK_ENTRIES];
        ShipHistoryData()
            : trackTurn(0)
        { }
    };

    void clearShipHistory(ShipHistoryData& d);
    ShipHistoryData::Track* adjustShipHistory(ShipHistoryData& d, int turn);
    ShipHistoryData::Track* getShipHistory(ShipHistoryData& d, int turn);

} }

#endif
