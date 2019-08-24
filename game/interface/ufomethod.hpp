/**
  *  \file game/interface/ufomethod.hpp
  */
#ifndef C2NG_GAME_INTERFACE_UFOMETHOD_HPP
#define C2NG_GAME_INTERFACE_UFOMETHOD_HPP

#include "game/map/ufo.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    enum UfoMethod {
        iumMark,
        iumUnmark
    };

    void callUfoMethod(game::map::Ufo& ufo, UfoMethod ium, interpreter::Arguments& args);

} }

#endif
