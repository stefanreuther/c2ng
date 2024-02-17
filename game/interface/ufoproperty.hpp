/**
  *  \file game/interface/ufoproperty.hpp
  *  \brief Enum game::interface::UfoProperty
  */
#ifndef C2NG_GAME_INTERFACE_UFOPROPERTY_HPP
#define C2NG_GAME_INTERFACE_UFOPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/map/ufo.hpp"

namespace game { namespace interface {

    /** Ufo property identifier. */
    enum UfoProperty {
        iupColorEGA,
        iupColorPCC,
        iupHeadingInt,
        iupHeadingName,
        iupId,
        iupId2,
        iupInfo1,
        iupInfo2,
        iupKeepFlag,
        iupLastScan,
        iupLocX,
        iupLocY,
        iupMarked,
        iupMoveDX,
        iupMoveDY,
        iupName,
        iupRadius,
        iupSpeedInt,
        iupSpeedName,
        iupType,
        iupVisiblePlanet,
        iupVisibleShip
    };

    /** Get Ufo property.
        @param ufo   Ufo
        @param iup   Property identifier
        @param tx    Translator (for names)
        @param iface Interpreter Interface (for names)
        @return newly-allocated value */
    afl::data::Value* getUfoProperty(const game::map::Ufo& ufo, UfoProperty iup, afl::string::Translator& tx);

    /** Set Ufo property.
        @param ufo   Ufo
        @param iup   Property identifier
        @param value Value, owned by caller
        @throw interpreter::Error if property is not assignable */
    void setUfoProperty(game::map::Ufo& ufo, UfoProperty iup, const afl::data::Value* value);

} }

#endif
