/**
  *  \file test/util/unicodecharstest.cpp
  *  \brief Test for util::UnicodeChars
  */

#include "util/unicodechars.hpp"
#include "afl/test/testrunner.hpp"

/** Trivial test. */
AFL_TEST("util.UnicodeChars", a)
{
    // This is just a header file with macros.
    // Perform some basic sanity/validity checks.
    const char* p = UTF_BALLOT_CROSS;
    a.checkNonNull("01", p);

    p = UTF_DOWN_ARROW;
    a.checkNonNull("11", p);
}
