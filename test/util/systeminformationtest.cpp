/**
  *  \file test/util/systeminformationtest.cpp
  *  \brief Test for util::SystemInformation
  */

#include "util/systeminformation.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("util.SystemInformation", a)
{
    // Default values are sensible
    util::SystemInformation testee;
    a.check("01", testee.numProcessors > 0);
    a.check("02", !testee.operatingSystem.empty());

    // System values are sensible
    testee = util::getSystemInformation();
    a.check("11", testee.numProcessors > 0);
    a.check("12", !testee.operatingSystem.empty());
}
