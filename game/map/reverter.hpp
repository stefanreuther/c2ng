/**
  *  \file game/map/reverter.hpp
  */
#ifndef C2NG_GAME_MAP_REVERTER_HPP
#define C2NG_GAME_MAP_REVERTER_HPP

#include "afl/base/deletable.hpp"

namespace game { namespace map {

    class Reverter : public afl::base::Deletable {
        // FIXME: define interface
        // - determine number of structures to sell
        // - determine number of starship components to sell
        // - determine number of supplies to buy
        // - determine previous fcode
        // - determine previous mission
        // - prepare/execute location reset
    };

} }

#endif
