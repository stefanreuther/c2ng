/**
  *  \file game/interface/minefieldmethod.hpp
  *  \brief Enum game::interface::MinefieldMethod
  */
#ifndef C2NG_GAME_INTERFACE_MINEFIELDMETHOD_HPP
#define C2NG_GAME_INTERFACE_MINEFIELDMETHOD_HPP

#include "game/map/minefield.hpp"
#include "game/map/universe.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    /** Minefield method identifier. */
    enum MinefieldMethod {
        immMark,
        immUnmark,
        immDelete
    };

    /** Call a minefield method.
        @param [in,out] mf    Minefield
        @param [in]     imm   Method identifier
        @param [in]     args  Method arguments
        @param [in,out] univ  Universe */
    void callMinefieldMethod(game::map::Minefield& mf, MinefieldMethod imm, interpreter::Arguments& args, game::map::Universe& univ);

} }

#endif
