/**
  *  \file interpreter/expr/builtinfunction.hpp
  *  \brief Code Generation for Builtin Functions
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

    /** Look up descriptor for a builtin function.
        Builtin functions are directly encoded into the bytecode, and can thus not be redefined by the user.

        \param name Function name

        \return Descriptor, pointing to static storage. Null if this is not a builtin function.
        Call desc.generator(desc) to obtain a FunctionCallNode. */
    const BuiltinFunctionDescriptor* lookupBuiltinFunction(const String_t& name);

} }

#endif
