/**
  *  \file game/interface/ufoproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_UFOPROPERTY_HPP
#define C2NG_GAME_INTERFACE_UFOPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/map/ufo.hpp"

namespace game { namespace interface {

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

    afl::data::Value* getUfoProperty(const game::map::Ufo& ufo, UfoProperty iup,
                                     afl::string::Translator& tx,
                                     InterpreterInterface& iface);
    void setUfoProperty(game::map::Ufo& ufo, UfoProperty iup, afl::data::Value* value);

} }

#endif
