/**
  *  \file test/util/translationtest.cpp
  *  \brief Test for util::Translation
  */

#include "util/translation.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("util.Translation", a)
{
    // Nothing to test for now; just check well-formedness of header file.
    const char*const s = N_("str");
    a.checkNonNull("01", s);
}
