/**
  *  \file u/t_interpreter_typehint.cpp
  *  \brief Test for interpreter::TypeHint
  */

#include "interpreter/typehint.hpp"

#include "t_interpreter.hpp"

/** Test well-formedness of header file.
    This is an enum so we can't test much more than that. */
void
TestInterpreterTypeHint::testHeader()
{
    interpreter::TypeHint testee;
    (void) testee;
}

