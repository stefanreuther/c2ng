/**
  *  \file game/interface/drawingproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_DRAWINGPROPERTY_HPP
#define C2NG_GAME_INTERFACE_DRAWINGPROPERTY_HPP

#include "game/map/drawing.hpp"
#include "afl/data/value.hpp"

namespace game { namespace interface {

    /** Drawing property identifier. */
    enum DrawingProperty {
        idpColor,
        idpComment,
        idpEndX,
        idpEndY,
        idpExpire,
        idpLocX,
        idpLocY,
        idpRadius,
        idpShape,
        idpTag,
        idpTypeString,
        idpTypeCode
    };

    afl::data::Value* getDrawingProperty(const game::map::Drawing& d, DrawingProperty idp);
    void setDrawingProperty(game::map::Drawing& d, DrawingProperty idp, afl::data::Value* value);

} }

#endif
