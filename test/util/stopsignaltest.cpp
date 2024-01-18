/**
  *  \file test/util/stopsignaltest.cpp
  *  \brief Test for util::StopSignal
  */

#include "util/stopsignal.hpp"
#include "afl/test/testrunner.hpp"

/** Simple coverage test. */
AFL_TEST("util.StopSignal", a)
{
    util::StopSignal testee;
    a.check("01", !testee.get());

    testee.set();
    a.check("11", testee.get());

    testee.clear();
    a.check("21", !testee.get());
}
