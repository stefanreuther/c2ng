/**
  *  \file u/t_util_keymapinformation.cpp
  *  \brief Test for util::KeymapInformation
  */

#include "util/keymapinformation.hpp"

#include "t_util.hpp"

void
TestUtilKeymapInformation::testIt()
{
    // Create
    util::KeymapInformation testee;
    testee.add(0, "FOO");
    testee.add(2, "BAR");

    // Query
    TS_ASSERT_EQUALS(testee.size(), 2U);
    TS_ASSERT_EQUALS(testee.find("BAR"), 1U);
    TS_ASSERT_EQUALS(testee.find("BAZ"), util::KeymapInformation::nil);
    TS_ASSERT_EQUALS(testee.find("bar"), util::KeymapInformation::nil);

    // Get entry, success
    size_t level;
    String_t name;
    TS_ASSERT_EQUALS(testee.get(1, level, name), true);
    TS_ASSERT_EQUALS(level, 2U);
    TS_ASSERT_EQUALS(name, "BAR");

    // Get entry, failure
    TS_ASSERT_EQUALS(testee.get(2, level, name), false);

    // Clear; verify
    testee.clear();
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT_EQUALS(testee.get(1, level, name), false);
    TS_ASSERT_EQUALS(testee.find("BAR"), util::KeymapInformation::nil);
}
