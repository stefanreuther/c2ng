/**
  *  \file game/interface/ufomethod.hpp
  *  \brief Enum game::interface::UfoMethod
  */
#ifndef C2NG_GAME_INTERFACE_UFOMETHOD_HPP
#define C2NG_GAME_INTERFACE_UFOMETHOD_HPP

#include "game/map/ufo.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    /** Ufo method identifier. */
    enum UfoMethod {
        iumMark,
        iumUnmark
    };

    /** Call Ufo method.
        @param ufo  Ufo
        @param ium  Method identifier
        @param args Parameters */
    void callUfoMethod(game::map::Ufo& ufo, UfoMethod ium, interpreter::Arguments& args);

} }

#endif
