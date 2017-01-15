/**
  *  \file game/interface/drawingmethod.hpp
  */
#ifndef C2NG_GAME_INTERFACE_DRAWINGMETHOD_HPP
#define C2NG_GAME_INTERFACE_DRAWINGMETHOD_HPP

#include "game/map/drawing.hpp"
#include "game/map/drawingcontainer.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    enum DrawingMethod {
        idmDelete,
        idmSetComment,
        idmSetColor
    };

    void callDrawingMethod(game::map::DrawingContainer& container,
                           game::map::DrawingContainer::Iterator_t it,
                           DrawingMethod method,
                           interpreter::Arguments& args);

} }

#endif
