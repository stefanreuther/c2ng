/**
  *  \file test/interpreter/typehinttest.cpp
  *  \brief Test for interpreter::TypeHint
  */

#include "interpreter/typehint.hpp"
#include "afl/test/testrunner.hpp"

/** Test well-formedness of header file.
    This is an enum so we can't test much more than that. */
AFL_TEST_NOARG("interpreter.TypeHint")
{
    interpreter::TypeHint testee;
    (void) testee;
}
