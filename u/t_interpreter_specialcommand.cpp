/**
  *  \file u/t_interpreter_specialcommand.cpp
  *  \brief Test for interpreter::SpecialCommand
  */

#include "interpreter/specialcommand.hpp"

#include "t_interpreter.hpp"

/** Interface test. */
void
TestInterpreterSpecialCommand::testInterface()
{
    class Tester : public interpreter::SpecialCommand {
     public:
        virtual void compileCommand(interpreter::Tokenizer& /*line*/, interpreter::BytecodeObject& /*bco*/, const interpreter::StatementCompilationContext& /*scc*/)
            { }
    };
    Tester t;
}

