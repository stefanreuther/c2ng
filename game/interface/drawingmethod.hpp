/**
  *  \file game/interface/drawingmethod.hpp
  *  \brief Enum game::interface::DrawingMethod
  */
#ifndef C2NG_GAME_INTERFACE_DRAWINGMETHOD_HPP
#define C2NG_GAME_INTERFACE_DRAWINGMETHOD_HPP

#include "game/map/drawing.hpp"
#include "game/map/drawingcontainer.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    /** Method to execute on a Drawing. */
    enum DrawingMethod {
        idmDelete,
        idmSetComment,
        idmSetColor
    };

    /** Invoke method on Drawing.
        @param container   Drawing Container
        @param it          Iterator into drawing container, identifies the target drawing
        @param method      Method to invoke
        @param args        Arguments */
    void callDrawingMethod(game::map::DrawingContainer& container,
                           game::map::DrawingContainer::Iterator_t it,
                           DrawingMethod method,
                           interpreter::Arguments& args);

} }

#endif
