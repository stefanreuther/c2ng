/**
  *  \file game/interface/explosionproperty.hpp
  *  \brief Enum game::interface::ExplosionProperty
  */
#ifndef C2NG_GAME_INTERFACE_EXPLOSIONPROPERTY_HPP
#define C2NG_GAME_INTERFACE_EXPLOSIONPROPERTY_HPP

#include "afl/data/value.hpp"
#include "afl/string/translator.hpp"
#include "game/map/explosion.hpp"

namespace game { namespace interface {

    /** Property for a game::map::Explosion. */
    enum ExplosionProperty {
        iepId,
        iepShipId,
        iepShipName,
        iepLocX,
        iepLocY,
        iepName,
        iepTypeStr,
        iepTypeChar
    };

    /** Get property of an explosion.
        @param expl  Explosion
        @param iep   Property
        @param tx    Translator (for names)
        @param iface Interface (for names) */
    afl::data::Value* getExplosionProperty(const game::map::Explosion& expl,
                                           ExplosionProperty iep,
                                           afl::string::Translator& tx,
                                           InterpreterInterface& iface);

} }

#endif
