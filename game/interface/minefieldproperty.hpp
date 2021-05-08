/**
  *  \file game/interface/minefieldproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_MINEFIELDPROPERTY_HPP
#define C2NG_GAME_INTERFACE_MINEFIELDPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/map/minefield.hpp"

namespace game { namespace interface {

    /** Minefield property identifier. */
    enum MinefieldProperty {
        impId,
        impLastScan,
        impLocX,
        impLocY,
        impMarked,
        impRadius,
        impScanType,
        impTypeCode,
        impTypeStr,
        impUnits
    };

    afl::data::Value* getMinefieldProperty(const game::map::Minefield& mf, MinefieldProperty imp);
    void setMinefieldProperty(game::map::Minefield& mf, MinefieldProperty imp, const afl::data::Value* value);

} }

#endif
