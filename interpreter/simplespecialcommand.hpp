/**
  *  \file interpreter/simplespecialcommand.hpp
  *  \brief Class interpreter::SimpleSpecialCommand
  */
#ifndef C2NG_INTERPRETER_SIMPLESPECIALCOMMAND_HPP
#define C2NG_INTERPRETER_SIMPLESPECIALCOMMAND_HPP

#include "interpreter/specialcommand.hpp"

namespace interpreter {

    /** Simple special command.
        This hands compilation to a static function. */
    class SimpleSpecialCommand : public SpecialCommand {
     public:
        /** Shortcut for compilation function. */
        typedef void (*Compile_t)(Tokenizer& line, BytecodeObject& bco, const StatementCompilationContext& scc);

        /** Constructor.
            \param function Function to compile the special command. */
        SimpleSpecialCommand(Compile_t function);

        /** Destructor. */
        ~SimpleSpecialCommand();

        // SpecialCommand:
        void compileCommand(Tokenizer& line, BytecodeObject& bco, const StatementCompilationContext& scc);

     private:
        const Compile_t m_function;
    };

}

#endif
