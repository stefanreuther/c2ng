/**
  *  \file u/t_util_systeminformation.cpp
  *  \brief Test for util::SystemInformation
  */

#include "util/systeminformation.hpp"

#include "t_util.hpp"

void
TestUtilSystemInformation::testIt()
{
    // Default values are sensible
    util::SystemInformation testee;
    TS_ASSERT(testee.numProcessors > 0);
    TS_ASSERT(!testee.operatingSystem.empty());

    // System values are sensible
    testee = util::getSystemInformation();
    TS_ASSERT(testee.numProcessors > 0);
    TS_ASSERT(!testee.operatingSystem.empty());
}

