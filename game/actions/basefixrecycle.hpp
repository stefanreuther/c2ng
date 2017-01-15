/**
  *  \file game/actions/basefixrecycle.hpp
  */
#ifndef C2NG_GAME_ACTIONS_BASEFIXRECYCLE_HPP
#define C2NG_GAME_ACTIONS_BASEFIXRECYCLE_HPP

#include "game/map/planet.hpp"
#include "game/types.hpp"
#include "afl/bits/smallset.hpp"
#include "game/map/ship.hpp"

namespace game { namespace actions {

    class BaseFixRecycle {
     public:
        typedef afl::bits::SmallSet<ShipyardAction> ShipyardActionSet_t;

        explicit BaseFixRecycle(game::map::Planet& planet);

        ShipyardActionSet_t getValidActions(const game::map::Ship& sh) const;

        bool set(ShipyardAction action, game::map::Ship* ship);

     private:
        game::map::Planet& m_planet;
    };

} }

#endif
