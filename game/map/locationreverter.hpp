/**
  *  \file game/map/locationreverter.hpp
  *  \brief Class game::map::LocationReverter
  */
#ifndef C2NG_GAME_MAP_LOCATIONREVERTER_HPP
#define C2NG_GAME_MAP_LOCATIONREVERTER_HPP

#include "afl/base/deletable.hpp"
#include "game/ref/list.hpp"
#include "afl/bits/smallset.hpp"

namespace game { namespace map {

    /** Reset location.
        Location reset will reset (parts) of all units at a given location to their previous values.
        Because cargo can be transferred between units at a location, they can be reverted only as a group.

        This class contains a prepared Reset action.
        Use as:
        - use Reverter::createLocationReverter() to create instance
        - examine getAffectedObjects(), getAvailableModes()
        - call commit()

        The underlying turn should not be structurally modified (i.e. new results loaded or unloaded) while the LocationReverter is active. */
    class LocationReverter : public afl::base::Deletable {
     public:
        /** Reset mode. */
        enum Mode {
            /** Reset missions, names, friendly codes. */
            Missions,

            /** Reset cargo and everything that can be bought for cargo. */
            Cargo
        };

        /** Set of modes. */
        typedef afl::bits::SmallSet<Mode> Modes_t;

        /** Get list of affected objects.
            \return list */
        virtual game::ref::List getAffectedObjects() const = 0;

        /** Get available modes.
            Some modes may be unavailable at some time.
            \return available modes */
        virtual Modes_t getAvailableModes() const = 0;

        /** Execute.

            \param modes Modes. Must be subset of getAvailableModes().

            \throw std::exception on error.
            commit() need not be able to handle structural modifications that happen
            while the LocationReverter is alive (e.g. a starbase added or removed).
            If it detects such, it can throw an exception and need not back out everything.
            This will not happen normally. */
        virtual void commit(Modes_t modes) = 0;
    };

} }

#endif
