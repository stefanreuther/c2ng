/**
  *  \file test/util/syntax/formattest.cpp
  *  \brief Test for util::syntax::Format
  */

#include "util/syntax/format.hpp"
#include "afl/test/testrunner.hpp"

/** Test well-formedness of header file.
    This is just an enum so we can't test much more than that. */
AFL_TEST_NOARG("util.syntax.Format")
{
    util::syntax::Format t = util::syntax::CommentFormat;
    (void) t;
}
