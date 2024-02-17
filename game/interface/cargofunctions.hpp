/**
  *  \file game/interface/cargofunctions.hpp
  *  \brief Cargo-related script functions
  */
#ifndef C2NG_GAME_INTERFACE_CARGOFUNCTIONS_HPP
#define C2NG_GAME_INTERFACE_CARGOFUNCTIONS_HPP

#include "game/cargospec.hpp"
#include "afl/data/value.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    /** Check cargospec argument.
        \param out   [out] Result will be placed here
        \param value [in] Value given by user
        \return true if value was specified, false if value was null (out not changed)
        \throw Error if value is invalid */
    bool checkCargoSpecArg(CargoSpec& out, const afl::data::Value* value);

    // Script function implementations
    afl::data::Value* IFCAdd(interpreter::Arguments& args);
    afl::data::Value* IFCCompare(interpreter::Arguments& args);
    afl::data::Value* IFCDiv(interpreter::Arguments& args);
    afl::data::Value* IFCExtract(interpreter::Arguments& args);
    afl::data::Value* IFCMul(interpreter::Arguments& args);
    afl::data::Value* IFCRemove(interpreter::Arguments& args);
    afl::data::Value* IFCSub(interpreter::Arguments& args);

} }

#endif
