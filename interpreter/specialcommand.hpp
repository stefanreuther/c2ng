/**
  *  \file interpreter/specialcommand.hpp
  *  \brief Interface interpreter::SpecialCommand
  */
#ifndef C2NG_INTERPRETER_SPECIALCOMMAND_HPP
#define C2NG_INTERPRETER_SPECIALCOMMAND_HPP

#include "afl/base/deletable.hpp"

namespace interpreter {

    class Tokenizer;
    class BytecodeObject;
    class StatementCompilationContext;

    /** Interface for a special commands.
        Special commands are compiled specially.
        They are looked up in a special symbol table.
        Objects of this type are made known to the compiler using World instance.
        For the user, special commands behave identical to commands implemented within the compiler core. */
    class SpecialCommand : public afl::base::Deletable {
     public:
        virtual void compileCommand(Tokenizer& line, BytecodeObject& bco, const StatementCompilationContext& scc) = 0;
    };

}

#endif
