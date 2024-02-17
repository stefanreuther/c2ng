/**
  *  \file game/interface/vcrproperty.hpp
  *  \brief Enum game::interface::VcrProperty
  */
#ifndef C2NG_GAME_INTERFACE_VCRPROPERTY_HPP
#define C2NG_GAME_INTERFACE_VCRPROPERTY_HPP

#include "afl/data/value.hpp"
#include "afl/string/translator.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/vcr/database.hpp"

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
        @param tx             Translator
        @param root           Root (for players)
        @param battles        Battles
        @param shipList       Ship list (for component names, battle outcome) */
    afl::data::Value* getVcrProperty(size_t battleNumber,
                                     VcrProperty ivp,
                                     afl::string::Translator& tx,
                                     const afl::base::Ref<const Root>& root,
                                     const afl::base::Ptr<game::vcr::Database>& battles,
                                     const afl::base::Ref<const game::spec::ShipList>& shipList);

} }

#endif
