/**
  *  \file test/util/keymapinformationtest.cpp
  *  \brief Test for util::KeyMapInformation
  */

#include "util/keymapinformation.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("util.KeyMapInformation", a)
{
    // Create
    util::KeymapInformation testee;
    testee.add(0, "FOO");
    testee.add(2, "BAR");

    // Query
    a.checkEqual("01", testee.size(), 2U);
    a.checkEqual("02", testee.find("BAR"), 1U);
    a.checkEqual("03", testee.find("BAZ"), util::KeymapInformation::nil);
    a.checkEqual("04", testee.find("bar"), util::KeymapInformation::nil);

    // Get entry, success
    size_t level;
    String_t name;
    a.checkEqual("11", testee.get(1, level, name), true);
    a.checkEqual("12", level, 2U);
    a.checkEqual("13", name, "BAR");

    // Get entry, failure
    a.checkEqual("21", testee.get(2, level, name), false);

    // Clear; verify
    testee.clear();
    a.checkEqual("31", testee.size(), 0U);
    a.checkEqual("32", testee.get(1, level, name), false);
    a.checkEqual("33", testee.find("BAR"), util::KeymapInformation::nil);
}
