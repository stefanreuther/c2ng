/**
  *  \file u/t_util_keymap.cpp
  *  \brief Test for util::Keymap
  */

#include "util/keymap.hpp"

#include "t_util.hpp"
#include "util/keymapinformation.hpp"

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

    TS_ASSERT_EQUALS(a.getNumDirectParents(), 0U);
    TS_ASSERT_EQUALS(b.getNumDirectParents(), 1U);

    TS_ASSERT_EQUALS(b.getDirectParent(0), &a);
    TS_ASSERT_EQUALS(b.getDirectParent(1), (util::Keymap*) 0);

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

    // Look up, asking for place of definition
    util::KeymapRef_t where;
    b.lookupCommand(1, where);
    TS_ASSERT_EQUALS(where, &b);
    b.lookupCommand(4, where);
    TS_ASSERT_EQUALS(where, &a);

    // Look up conditions
    TS_ASSERT_EQUALS(a.lookupCondition(1), 3U);
    TS_ASSERT_EQUALS(a.lookupCondition(4), 6U);
    TS_ASSERT_EQUALS(a.lookupCondition(7), 0U);
    TS_ASSERT_EQUALS(a.lookupCondition(99), 0U);
    TS_ASSERT_EQUALS(b.lookupCondition(1), 5U);
    TS_ASSERT_EQUALS(b.lookupCondition(4), 6U);
    TS_ASSERT_EQUALS(b.lookupCondition(7), 9U);
    TS_ASSERT_EQUALS(b.lookupCondition(99), 0U);
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

    a.addKey(1, 2, 4);
    TS_ASSERT(a.isChanged());
    a.markChanged(false);
}

/** Test describe(). */
void
TestUtilKeymap::testDescribe()
{
    util::Keymap a("A");
    util::Keymap a1("A1");
    util::Keymap a1b("A1B");
    util::Keymap a2("A2");
    a.addParent(a1);
    a1.addParent(a1b);
    a.addParent(a2);

    // Describe A with big limit
    {
        util::KeymapInformation info;
        a.describe(info, 99);

        TS_ASSERT_EQUALS(info.size(), 4U);

        size_t level;
        String_t name;
        TS_ASSERT_EQUALS(info.get(0, level, name), true);
        TS_ASSERT_EQUALS(level, 0U);
        TS_ASSERT_EQUALS(name, "A");

        TS_ASSERT_EQUALS(info.get(1, level, name), true);
        TS_ASSERT_EQUALS(level, 1U);
        TS_ASSERT_EQUALS(name, "A1");

        TS_ASSERT_EQUALS(info.get(2, level, name), true);
        TS_ASSERT_EQUALS(level, 2U);
        TS_ASSERT_EQUALS(name, "A1B");

        TS_ASSERT_EQUALS(info.get(3, level, name), true);
        TS_ASSERT_EQUALS(level, 1U);
        TS_ASSERT_EQUALS(name, "A2");
    }

    // Describe A with low limit
    {
        util::KeymapInformation info;
        a.describe(info, 1);

        TS_ASSERT_EQUALS(info.size(), 4U);

        size_t level;
        String_t name;
        TS_ASSERT_EQUALS(info.get(0, level, name), true);
        TS_ASSERT_EQUALS(level, 0U);
        TS_ASSERT_EQUALS(name, "A");

        TS_ASSERT_EQUALS(info.get(1, level, name), true);
        TS_ASSERT_EQUALS(level, 1U);
        TS_ASSERT_EQUALS(name, "A1");

        TS_ASSERT_EQUALS(info.get(2, level, name), true);
        TS_ASSERT_EQUALS(level, 2U);     // placeholder
        TS_ASSERT_EQUALS(name, "");

        TS_ASSERT_EQUALS(info.get(3, level, name), true);
        TS_ASSERT_EQUALS(level, 1U);
        TS_ASSERT_EQUALS(name, "A2");
    }

    // Describe A with very low limit
    {
        util::KeymapInformation info;
        a.describe(info, 0);

        TS_ASSERT_EQUALS(info.size(), 2U);

        size_t level;
        String_t name;
        TS_ASSERT_EQUALS(info.get(0, level, name), true);
        TS_ASSERT_EQUALS(level, 0U);
        TS_ASSERT_EQUALS(name, "A");

        TS_ASSERT_EQUALS(info.get(1, level, name), true);
        TS_ASSERT_EQUALS(level, 1U);     // placeholder
        TS_ASSERT_EQUALS(name, "");
    }
}

/** Test describe() with multiple inheritance. */
void
TestUtilKeymap::testDescribeMI()
{
    util::Keymap base("BASE"), left("LEFT"), right("RIGHT"), common("COMMON");
    common.addParent(left);
    common.addParent(right);
    left.addParent(base);
    right.addParent(base);

    // Describe COMMON
    util::KeymapInformation info;
    common.describe(info, 99);

    TS_ASSERT_EQUALS(info.size(), 4U);

    size_t level;
    String_t name;
    TS_ASSERT_EQUALS(info.get(0, level, name), true);
    TS_ASSERT_EQUALS(level, 0U);
    TS_ASSERT_EQUALS(name, "COMMON");

    TS_ASSERT_EQUALS(info.get(1, level, name), true);
    TS_ASSERT_EQUALS(level, 1U);
    TS_ASSERT_EQUALS(name, "LEFT");

    TS_ASSERT_EQUALS(info.get(2, level, name), true);
    TS_ASSERT_EQUALS(level, 2U);
    TS_ASSERT_EQUALS(name, "BASE");

    TS_ASSERT_EQUALS(info.get(3, level, name), true);
    TS_ASSERT_EQUALS(level, 1U);
    TS_ASSERT_EQUALS(name, "RIGHT");
}

