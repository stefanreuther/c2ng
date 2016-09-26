/**
  *  \file u/t_util_unicodechars.cpp
  *  \brief Test for util::UnicodeChars
  */

#include "util/unicodechars.hpp"

#include "t_util.hpp"

/** Trivial test. */
void
TestUtilUnicodeChars::testIt()
{
    // This is just a header file with macros.
    // Perform some basic sanity/validity checks.
    const char* p = UTF_BALLOT_CROSS;
    TS_ASSERT(p != 0);

    p = UTF_DOWN_ARROW;
    TS_ASSERT(p != 0);
}

