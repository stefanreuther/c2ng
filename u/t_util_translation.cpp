/**
  *  \file u/t_util_translation.cpp
  *  \brief Test for util::Translation
  */

#include "util/translation.hpp"

#include "t_util.hpp"

void
TestUtilTranslation::testIt()
{
    // Nothing to test for now; just check well-formedness of header file.
    const char*const s = N_("str");
    TS_ASSERT(s != 0);
}

