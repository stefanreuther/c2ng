/**
  *  \file u/t_util_stopsignal.cpp
  *  \brief Test for util::StopSignal
  */

#include "util/stopsignal.hpp"

#include "t_util.hpp"

/** Simple coverage test. */
void
TestUtilStopSignal::testIt()
{
    util::StopSignal testee;
    TS_ASSERT(!testee.get());

    testee.set();
    TS_ASSERT(testee.get());

    testee.clear();
    TS_ASSERT(!testee.get());
}

