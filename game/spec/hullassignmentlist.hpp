/**
  *  \file game/spec/hullassignmentlist.hpp
  *  \brief Class game::spec::HullAssignmentList
  */
#ifndef C2NG_GAME_SPEC_HULLASSIGNMENTLIST_HPP
#define C2NG_GAME_SPEC_HULLASSIGNMENTLIST_HPP

#include <vector>
#include "game/config/hostconfiguration.hpp"
#include "game/playerset.hpp"

namespace game { namespace spec {

    /** Hull assignment list (truehull).
        This stores a mapping of players and slot numbers (positions) to hull numbers
        and allows forward and reverse queries.

        Details can be configured (setMode()) to match host configurations. */
    class HullAssignmentList {
     public:
        /** Access mode. */
        enum Mode {
            /** Player-indexed mode.
                Player numbers are actual player numbers.
                This is the default. */
            PlayerIndexed,
            /** Race-indexed mode.
                Player numbers are actually race numbers and are indexed through PlayerRace.
                This is PHost's MapTruehullByPlayerRace mode. */
            RaceIndexed
        };

        /** Default constructor.
            Makes an empty mapping. */
        HullAssignmentList();

        /** Destructor. */
        ~HullAssignmentList();

        /** Clear.
            Resets the object into its default state. */
        void clear();

        /** Set access mode.
            \param mode New mode */
        void setMode(Mode mode);

        /** Add a mapping.
            If parameters are out of range, the call is ignored.
            (This means it is not possible to reset a populated position to 0!)
            \param player Player number (>0)
            \param position Slot number (>0)
            \param hullNr Hull number (>0) */
        void add(int player, int position, int hullNr);

        /** Clear a player slot.
            \param player Player number (>0) */
        void clearPlayer(int player);

        /** Get index, given a hull.
            \param config Configuration
            \param player Player number (>0)
            \param hullNr Hull number (>0)
            \return Index such that getHullFromIndex(config, player, X) == hullNr. Zero if no such index exists
            or parameters are out of range (player cannot build this hull). */
        int getIndexFromHull(const game::config::HostConfiguration& config, int player, int hullNr) const;

        /** Get hull, given an index.
            \param config Configuration
            \param player Player number (>0)
            \param index Index [1, getMaxIndex(config,player)]
            \return Hull number. Zero if parameters are out of range, or the player has no available hull in this slot. */
        int getHullFromIndex(const game::config::HostConfiguration& config, int player, int index) const;

        /** Get maximum index.
            Returns the maximum index that makes sense to pass to getHullFromIndex().
            getHullFromIndex() will return 0 for all indexes strictly greater than the return value of this function.
            \param config Configuration
            \param player Player number (>0)
            \return Maximum index (inclusive) */
        int getMaxIndex(const game::config::HostConfiguration& config, int player) const;

        /** Get set of players that can build a hull.
            \param config Configuration
            \param hullNr Hull number
            \return Set of players such that for each set player, getIndexFromHull() is nonzero. */
        PlayerSet_t getPlayersForHull(const game::config::HostConfiguration& config, int hullNr) const;

     private:
        /** Access mode. */
        Mode m_mode;

        /** Mapping; first by player, then by index.
            Note that as of 20170412, we include the unused 0th element in both dimensions. */
        std::vector<std::vector<int> > m_mapping;

        int mapPlayer(const game::config::HostConfiguration& config, int player) const;
    };

} }

#endif
