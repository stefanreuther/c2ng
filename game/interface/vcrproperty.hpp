/**
  *  \file game/interface/vcrproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_VCRPROPERTY_HPP
#define C2NG_GAME_INTERFACE_VCRPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/vcr/battle.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "game/root.hpp"

namespace game { namespace interface {

    /** Property of a VCR record. */
    enum VcrProperty {
        ivpSeed,
        ivpMagic,
        ivpType,
        ivpAlgorithm,
        ivpFlags,
        ivpNumUnits,
        ivpUnits,
        ivpLocX,
        ivpLocY,
        ivpAmbient
    };

    afl::data::Value* getVcrProperty(size_t battleNumber,
                                     VcrProperty ivp,
                                     Session& session,
                                     afl::base::Ref<Root> root,     // for PlayerList
                                     afl::base::Ref<Turn> turn,     // for Turn
                                     afl::base::Ref<game::spec::ShipList> shipList);

} }

#endif
