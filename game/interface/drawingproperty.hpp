/**
  *  \file game/interface/drawingproperty.hpp
  *  \brief Drawing Properties
  */
#ifndef C2NG_GAME_INTERFACE_DRAWINGPROPERTY_HPP
#define C2NG_GAME_INTERFACE_DRAWINGPROPERTY_HPP

#include "afl/charset/charset.hpp"
#include "afl/data/value.hpp"
#include "game/map/drawing.hpp"

namespace game { namespace interface {

    /** Drawing property identifier. */
    enum DrawingProperty {
        idpColor,
        idpComment,
        idpEncodedMessage,
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

    /** Get property of a drawing.
        \param d Drawing
        \param idp Property
        \param charset Character set
        \return newly-allocated property value */
    afl::data::Value* getDrawingProperty(const game::map::Drawing& d, DrawingProperty idp, afl::charset::Charset& charset);

    /** Set property of a drawing.
        \param d Drawing
        \param idp Property
        \param value property value, owned by caller */
    void setDrawingProperty(game::map::Drawing& d, DrawingProperty idp, const afl::data::Value* value);

} }

#endif
