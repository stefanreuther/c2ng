/**
  *  \file game/interface/minefieldproperty.hpp
  *  \brief Enum game::interface::MinefieldProperty
  */
#ifndef C2NG_GAME_INTERFACE_MINEFIELDPROPERTY_HPP
#define C2NG_GAME_INTERFACE_MINEFIELDPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/map/minefield.hpp"

namespace game { namespace interface {

    /** Minefield property identifier. */
    enum MinefieldProperty {
        impId,
        impEncodedMessage,
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

    /** Get minefield property.
        @param mf  Minefield
        @param imp Property to query
        @return value, newly-allocated */
    afl::data::Value* getMinefieldProperty(const game::map::Minefield& mf, MinefieldProperty imp);

    /** Set minefield property.
        @param mf    Minefield
        @param imp   Property to set
        @param value Value, owned by caller
        @throw interpreter::Error */
    void setMinefieldProperty(game::map::Minefield& mf, MinefieldProperty imp, const afl::data::Value* value);

} }

#endif
