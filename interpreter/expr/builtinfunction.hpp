/**
  *  \file interpreter/expr/builtinfunction.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_BUILTINFUNCTION_HPP
#define C2NG_INTERPRETER_EXPR_BUILTINFUNCTION_HPP

#include "afl/string/string.hpp"
#include "afl/base/types.hpp"

namespace interpreter { namespace expr {

    class FunctionCallNode;

    /** Description of a builtin function. */
    struct BuiltinFunctionDescriptor {
        /// Name of the function.
        const char* name;

        /// Argument count restrictions.
        uint32_t min_args, max_args;

        /// Node generator function.
        FunctionCallNode* (*generator)(const BuiltinFunctionDescriptor& desc);

        /// Additional parameter for node generator.
        uint8_t generator_arg;
    };

    const BuiltinFunctionDescriptor* lookupBuiltinFunction(String_t name);

} }

#endif
