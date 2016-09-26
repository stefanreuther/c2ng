/**
  *  \file interpreter/typehint.hpp
  */
#ifndef C2NG_INTERPRETER_TYPEHINT_HPP
#define C2NG_INTERPRETER_TYPEHINT_HPP

namespace interpreter {

    /** Type.
        This is a hint for reflection, not an unconditional promise,
        that is, code generation may not rely on it, and all code must handle receiving a "wrong" value.
        However, it is intended to be close. */
    enum TypeHint {
        thNone,
        thBool,
        thInt,
        thFloat,
        thString,
        thProcedure,
        thFunction,
        thArray
    };

}

#endif
