/**
  *  \file game/interface/vcrproperty.hpp
  *  \brief Enum game::interface::VcrProperty
  */
#ifndef C2NG_GAME_INTERFACE_VCRPROPERTY_HPP
#define C2NG_GAME_INTERFACE_VCRPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "game/vcr/battle.hpp"

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

    /** Get property of a VCR record.
        @param battleNumber   Battle number, index into game::vcr::Database::getBattle()
        @param ivp            Property to query
        @param session        Session
        @param root           Root (for players)
        @param turn           Turn (for battles)
        @param shipList       Ship list (for unit names) */

    afl::data::Value* getVcrProperty(size_t battleNumber,
                                     VcrProperty ivp,
                                     Session& session,
                                     afl::base::Ref<const Root> root,
                                     afl::base::Ref<const Turn> turn,
                                     afl::base::Ref<const game::spec::ShipList> shipList);

} }

#endif
