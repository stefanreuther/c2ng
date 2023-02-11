/**
  *  \file game/map/shiphistorydata.hpp
  *  \brief Class game::map::ShipHistoryData
  */
#ifndef C2NG_GAME_MAP_SHIPHISTORYDATA_HPP
#define C2NG_GAME_MAP_SHIPHISTORYDATA_HPP

#include "game/map/point.hpp"
#include "game/types.hpp"

namespace game { namespace map {

    const size_t NUM_SHIP_TRACK_ENTRIES = 10;

    /** Ship history data.
        Represents a ship's history data, but does not interpret it further. */
    struct ShipHistoryData {
        /** Ship position record.
            Represents one scan of a ship.
            All values can be unknown. */
        struct Track {
            IntegerProperty_t x;              ///< Position, x.
            IntegerProperty_t y;              ///< Position, y.
            IntegerProperty_t speed;          ///< Warp factor.
            IntegerProperty_t heading;        ///< Heading.
            IntegerProperty_t mass;           ///< Mass.
        };

        /** Base turn.
            Oldest turn number stored in this entry. */
        int trackTurn;

        /** Number of ship position records.
            Index 0 is newest (trackTurn), 1 is turn before that, and so on. */
        Track track[NUM_SHIP_TRACK_ENTRIES];

        /** Constructor. */
        ShipHistoryData()
            : trackTurn(0)
        { }
    };

    /** Clear entire ship history data.
        All values will be set to defaults.
        @param [out] d Result */
    void clearShipHistory(ShipHistoryData& d);

    /** Clear single ship history entry.
        Clears the entry for the given turn. If the turn does not exist, does nothing.
        @param [in,out] d     ShipHistoryData object
        @param [in]     turn  Turn number */
    void clearShipHistory(ShipHistoryData& d, int turn);

    /** Adjust ship history to include a given turn.
        If the turn is newer than the newest history entry, discard old entries to make this entry fit in.
        @param [in,out] d     ShipHistoryData object
        @param [in]     turn  Turn number
        @return Pointer to resulting Track entry; null of turn is too old */
    ShipHistoryData::Track* adjustShipHistory(ShipHistoryData& d, int turn);

    /** Get ship history entry for a turn.
        @param [in] d     ShipHistoryData object
        @param [in] turn  Turn number
        @return Pointer to resulting Track entry; null if entry is not contained in record (too old/too new) */
    const ShipHistoryData::Track* getShipHistory(const ShipHistoryData& d, int turn);

} }

#endif
