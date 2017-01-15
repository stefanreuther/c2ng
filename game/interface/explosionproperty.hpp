/**
  *  \file game/interface/explosionproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_EXPLOSIONPROPERTY_HPP
#define C2NG_GAME_INTERFACE_EXPLOSIONPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/map/explosion.hpp"

namespace game { namespace interface {

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

    afl::data::Value* getExplosionProperty(const game::map::Explosion& expl,
                                           ExplosionProperty iep,
                                           afl::string::Translator& tx,
                                           InterpreterInterface& iface);

} }

#endif
