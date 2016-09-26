/**
  *  \file game/interface/ionstormproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_IONSTORMPROPERTY_HPP
#define C2NG_GAME_INTERFACE_IONSTORMPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/map/ionstorm.hpp"

namespace game { namespace interface {

    /** Ion Storm property identifier. */
    enum IonStormProperty {
        iipClass,
        iipHeadingInt,
        iipHeadingName,
        iipId,
        iipLocX,
        iipLocY,
        iipMarked,
        iipName,
        iipRadius,
        iipSpeedInt,
        iipSpeedName,
        iipStatusFlag,
        iipStatusName,
        iipVoltage
    };

    afl::data::Value* getIonStormProperty(const game::map::IonStorm& ion, IonStormProperty iip, afl::string::Translator& tx, InterpreterInterface& iface);
    void setIonStormProperty(game::map::IonStorm& ion, IonStormProperty iip, afl::data::Value* value);

} }

#endif
