/**
  *  \file game/actions/basefixrecycle.hpp
  *  \brief Class game::actions::BaseFixRecycle
  */
#ifndef C2NG_GAME_ACTIONS_BASEFIXRECYCLE_HPP
#define C2NG_GAME_ACTIONS_BASEFIXRECYCLE_HPP

#include <vector>
#include "afl/bits/smallset.hpp"
#include "afl/string/translator.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/types.hpp"

namespace game { namespace actions {

    /** Shipyard Action Control.

        To use, construct with the starbase as parameter.
        Use methods to inquire valid actions, set() to set one.

        Note that NoShipyardAction will never be returned as a valid action,
        but is a valid parameter to set(). */
    class BaseFixRecycle {
     public:
        /** Set of Shipyard Actions. */
        typedef afl::bits::SmallSet<ShipyardAction> ShipyardActionSet_t;

        /** Constructor.
            \param planet Planet with starbase.
            \param tx Translator (for error message)
            \throw Planet does not fulfil preconditions */
        BaseFixRecycle(game::map::Planet& planet, afl::string::Translator& tx);

        /** Get valid actions for a ship.
            \param ship Ship to check
            \return Set of actions that are valid for this ship */
        ShipyardActionSet_t getValidActions(const game::map::Ship& sh) const;

        /** Get valid actions.
            \param univ Universe
            \return Set of actions for which getValidShipIds() returns a nonempty result */
        ShipyardActionSet_t getValidActions(const game::map::Universe& univ) const;

        /** Get valid ships.
            \param univ Universe
            \param action Action to check
            \return List of ship Ids that are valid for the given action */
        std::vector<Id_t> getValidShipIds(const game::map::Universe& univ, ShipyardAction action) const;

        /** Set action.

            NoShipyardAction can be set with any value for \c ship.
            Other actions can only be set if a valid ship is passed.

            \param action Action to set
            \param univ Universe
            \param ship Ship to use
            \return true on success */
        bool set(ShipyardAction action, game::map::Universe& univ, game::map::Ship* ship);

     private:
        game::map::Planet& m_planet;
    };

} }

#endif
