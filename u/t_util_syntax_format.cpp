/**
  *  \file u/t_util_syntax_format.cpp
  *  \brief Test for util::syntax::Format
  */

#include "util/syntax/format.hpp"

#include "t_util_syntax.hpp"

/** Test well-formedness of header file.
    This is just an enum so we can't test much more than that. */
void
TestUtilSyntaxFormat::testHeader()
{
    util::syntax::Format t = util::syntax::CommentFormat;
    (void) t;
}

