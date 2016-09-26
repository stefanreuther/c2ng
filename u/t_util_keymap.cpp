/**
  *  \file u/t_util_keymap.cpp
  *  \brief Test for util::Keymap
  */

#include "util/keymap.hpp"

#include "t_util.hpp"

/** General keymap test. */
void
TestUtilKeymap::testKeymap()
{
    // ex IntKeymapTestSuite::testKeymap (part)
    util::Keymap a("TESTKEYMAP");
    util::Keymap b("TESTCHILD");

    // Check parents
    TS_ASSERT(!a.hasParent(b));
    TS_ASSERT(!b.hasParent(a));
    TS_ASSERT(a.hasParent(a));
    TS_ASSERT(b.hasParent(b));
    TS_ASSERT_THROWS_NOTHING(b.addParent(a));
    TS_ASSERT(!a.hasParent(b));
    TS_ASSERT(b.hasParent(a));
    TS_ASSERT_THROWS(b.addParent(a), std::runtime_error);
    TS_ASSERT_THROWS(a.addParent(b), std::runtime_error);
    TS_ASSERT_THROWS(a.addParent(a), std::runtime_error);
    TS_ASSERT_THROWS(b.addParent(b), std::runtime_error);

    // Check keys
    a.addKey(1, 2, 3);
    a.addKey(4, 5, 6);
    b.addKey(1, 4, 5);
    b.addKey(7, 8, 9);
    TS_ASSERT_EQUALS(a.lookupCommand(1), 2U);
    TS_ASSERT_EQUALS(a.lookupCommand(4), 5U);
    TS_ASSERT_EQUALS(a.lookupCommand(7), 0U);
    TS_ASSERT_EQUALS(a.lookupCommand(99), 0U);
    TS_ASSERT_EQUALS(b.lookupCommand(1), 4U);
    TS_ASSERT_EQUALS(b.lookupCommand(4), 5U);
    TS_ASSERT_EQUALS(b.lookupCommand(7), 8U);
    TS_ASSERT_EQUALS(b.lookupCommand(99), 0U);
}

/** Test change tracking. */
void
TestUtilKeymap::testChange()
{
    util::Keymap a("TEST");
    TS_ASSERT(!a.isChanged());

    a.addKey(1, 2, 3);
    TS_ASSERT(a.isChanged());
    a.markChanged(false);

    a.addKey(1, 2, 3);
    TS_ASSERT(!a.isChanged());
}
