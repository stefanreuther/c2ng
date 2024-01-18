/**
  *  \file test/interpreter/specialcommandtest.cpp
  *  \brief Test for interpreter::SpecialCommand
  */

#include "interpreter/specialcommand.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("interpreter.SpecialCommand")
{
    class Tester : public interpreter::SpecialCommand {
     public:
        virtual void compileCommand(interpreter::Tokenizer& /*line*/, interpreter::BytecodeObject& /*bco*/, const interpreter::StatementCompilationContext& /*scc*/)
            { }
    };
    Tester t;
}
