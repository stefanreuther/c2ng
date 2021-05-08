/**
  *  \file u/t_util_stringlist.cpp
  *  \brief Test for util::StringList
  */

#include "util/stringlist.hpp"

#include "t_util.hpp"

/** Simple test. */
void
TestUtilStringList::testIt()
{
    util::StringList testee;
    int32_t i;
    String_t s;

    // Verify empty
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT(testee.empty());
    TS_ASSERT(!testee.get(0, i, s));
    TS_ASSERT(!testee.get(size_t(-1), i, s));
    TS_ASSERT(!testee.get(1000000, i, s));

    // Populate
    testee.add(23, "hi");
    testee.add(42, "ho");
    TS_ASSERT_EQUALS(testee.size(), 2U);
    TS_ASSERT(!testee.empty());

    // Verify populated
    TS_ASSERT(testee.get(0, i, s));
    TS_ASSERT_EQUALS(i, 23);
    TS_ASSERT_EQUALS(s, "hi");

    TS_ASSERT(testee.get(1, i, s));
    TS_ASSERT_EQUALS(i, 42);
    TS_ASSERT_EQUALS(s, "ho");

    TS_ASSERT(!testee.get(size_t(-1), i, s));
    TS_ASSERT(!testee.get(1000000, i, s));

    // Verify find
    size_t n;
    TS_ASSERT(!testee.find(0, n));
    TS_ASSERT(!testee.find(1, n));
    TS_ASSERT(testee.find(42, n));
    TS_ASSERT_EQUALS(n, 1U);

    // Add some more
    testee.add(3, "x");         // 2
    testee.add(1, "y");         // 3
    testee.add(4, "z");         // 4
    testee.add(1, "a");         // 5
    testee.add(5, "b");         // 6
    TS_ASSERT(testee.find(1, n));
    TS_ASSERT_EQUALS(n, 3U);    // first instance of 1
}

/** Test sort. */
void
TestUtilStringList::testSort()
{
    util::StringList testee;
    testee.add(1, "foo");
    testee.add(2, "bar");
    testee.add(3, "baz");
    testee.add(4, "qux");
    testee.sortAlphabetically();

    TS_ASSERT_EQUALS(testee.size(), 4U);

    int32_t id;
    String_t value;
    TS_ASSERT(testee.get(0, id, value));
    TS_ASSERT_EQUALS(id, 2);
    TS_ASSERT_EQUALS(value, "bar");

    TS_ASSERT(testee.get(1, id, value));
    TS_ASSERT_EQUALS(id, 3);
    TS_ASSERT_EQUALS(value, "baz");

    TS_ASSERT(testee.get(2, id, value));
    TS_ASSERT_EQUALS(id, 1);
    TS_ASSERT_EQUALS(value, "foo");

    TS_ASSERT(testee.get(3, id, value));
    TS_ASSERT_EQUALS(id, 4);
    TS_ASSERT_EQUALS(value, "qux");
}

/** Test copy, swap, clear. */
void
TestUtilStringList::testCopy()
{
    util::StringList a;
    a.add(1, "foo");
    a.add(2, "bar");
    TS_ASSERT_EQUALS(a.size(), 2U);

    util::StringList b(a);
    TS_ASSERT_EQUALS(b.size(), 2U);

    util::StringList c;
    TS_ASSERT_EQUALS(c.size(), 0U);

    a.swap(c);
    TS_ASSERT_EQUALS(c.size(), 2U);
    TS_ASSERT_EQUALS(a.size(), 0U);

    a = c;
    TS_ASSERT_EQUALS(c.size(), 2U);
    TS_ASSERT_EQUALS(a.size(), 2U);

    a.clear();
    TS_ASSERT_EQUALS(a.size(), 0U);
}
