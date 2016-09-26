/**
  *  \file interpreter/simplespecialcommand.hpp
  */
#ifndef C2NG_INTERPRETER_SIMPLESPECIALCOMMAND_HPP
#define C2NG_INTERPRETER_SIMPLESPECIALCOMMAND_HPP

#include "interpreter/specialcommand.hpp"

namespace interpreter {

    /** Simple special command.
        This hands compilation to a static function. */
    class SimpleSpecialCommand : public SpecialCommand {
     public:
        typedef void (*Compile_t)(Tokenizer& line, BytecodeObject& bco, const StatementCompilationContext& scc);

        SimpleSpecialCommand(Compile_t function);
        ~SimpleSpecialCommand();

        void compileCommand(Tokenizer& line, BytecodeObject& bco, const StatementCompilationContext& scc);

     private:
        Compile_t m_function;
    };

}

#endif
