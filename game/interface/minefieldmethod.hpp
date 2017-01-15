/**
  *  \file game/interface/minefieldmethod.hpp
  */
#ifndef C2NG_GAME_INTERFACE_MINEFIELDMETHOD_HPP
#define C2NG_GAME_INTERFACE_MINEFIELDMETHOD_HPP

#include "game/map/minefield.hpp"
#include "interpreter/arguments.hpp"
#include "game/map/universe.hpp"

namespace game { namespace interface {

    enum MinefieldMethod {
        immMark,
        immUnmark,
        immDelete
    };

    void callMinefieldMethod(game::map::Minefield& mf, MinefieldMethod imm, interpreter::Arguments& args, game::map::Universe& univ);

} }

#endif
