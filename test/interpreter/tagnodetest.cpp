/**
  *  \file test/interpreter/tagnodetest.cpp
  *  \brief Test for interpreter::TagNode
  */

#include "interpreter/tagnode.hpp"
#include "afl/test/testrunner.hpp"

/** Test well-formedness of header file.
    This is a structure so we can't test much more than that. */
AFL_TEST_NOARG("interpreter.TagNode")
{
    interpreter::TagNode testee;
    (void) testee;
}
