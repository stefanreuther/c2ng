/**
  *  \file game/interface/ionstormproperty.hpp
  *  \brief Enum game::interface::IonStormProperty
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
        iipParentId,
        iipRadius,
        iipSpeedInt,
        iipSpeedName,
        iipStatusFlag,
        iipStatusName,
        iipVoltage
    };

    /** Get ion storm property.
        @param ion  Storm
        @param iip  Property
        @param tx   Translator (for names)
        @return Newly-allocated value */
    afl::data::Value* getIonStormProperty(const game::map::IonStorm& ion, IonStormProperty iip, afl::string::Translator& tx);

    /** Set ion storm property.
        @param ion  Storm
        @param iip  Property
        @param value Value to set, owned by caller
        @throw interpreter::Error if not assignable */
    void setIonStormProperty(game::map::IonStorm& ion, IonStormProperty iip, const afl::data::Value* value);

} }

#endif
