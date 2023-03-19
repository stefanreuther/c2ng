/**
  *  \file u/t_util_expressionlist.cpp
  *  \brief Test for util::ExpressionList
  */

#include "util/expressionlist.hpp"

#include "t_util.hpp"

/** Test most access operations. */
void
TestUtilExpressionList::testAccess()
{
    util::ExpressionList testee;
    size_t pos = 0;

    // Verify initial state
    TS_ASSERT(testee.empty());
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT(testee.get(0) == 0);
    TS_ASSERT(!testee.findIndexForValue("v").get(pos));

    // Add some values
    testee.pushBackNew(new util::ExpressionList::Item("n1", "[f1]", "v1"));
    testee.pushBackNew(new util::ExpressionList::Item("n2", "[f2]", "v2"));
    testee.pushBackNew(new util::ExpressionList::Item("n", "[f]", "v"));
    testee.pushBackNew(new util::ExpressionList::Item("n3", "[f3]", "v3"));

    TS_ASSERT(!testee.empty());
    TS_ASSERT_EQUALS(testee.size(), 4U);
    TS_ASSERT(testee.get(0) != 0);
    TS_ASSERT(testee.get(3) != 0);
    TS_ASSERT(testee.get(4) == 0);

    TS_ASSERT_EQUALS(testee.get(0)->name, "n1");
    TS_ASSERT_EQUALS(testee.get(0)->flags, "[f1]");
    TS_ASSERT_EQUALS(testee.get(0)->value, "v1");

    TS_ASSERT(testee.findIndexForValue("v").get(pos));
    TS_ASSERT_EQUALS(pos, 2U);

    // Move to front
    testee.moveToFront(2);
    TS_ASSERT_EQUALS(testee.size(), 4U);
    TS_ASSERT_EQUALS(testee.get(0)->name, "n");
    TS_ASSERT(testee.findIndexForValue("v").get(pos));
    TS_ASSERT_EQUALS(pos, 0U);

    // Clear
    testee.clear();
    TS_ASSERT(testee.empty());
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT(testee.get(0) == 0);
    TS_ASSERT(!testee.findIndexForValue("v").get(pos));
}

/** Test LRU behaviour. */
void
TestUtilExpressionList::testLRU()
{
    util::ExpressionList testee;
    testee.pushFrontNew(new util::ExpressionList::Item("1", "[f1]", "v1"), 3);      // v1
    testee.pushFrontNew(new util::ExpressionList::Item("2", "[f2]", "v2"), 3);      // v2:v1
    testee.pushFrontNew(new util::ExpressionList::Item("3", "[f3]", "v3"), 3);      // v3:v2:v1
    testee.pushFrontNew(new util::ExpressionList::Item("1a", "[f1a]", "v1"), 3);    // v1a:v3:v2
    testee.pushFrontNew(new util::ExpressionList::Item("4", "[f4]", "v4"), 3);      // v4:v1a:v3
    testee.pushFrontNew(new util::ExpressionList::Item("4b", "[f4b]", "v4"), 3);    // v4b:v1a:v3

    TS_ASSERT_EQUALS(testee.size(), 3U);

    TS_ASSERT_EQUALS(testee.get(0)->name, "4b");
    TS_ASSERT_EQUALS(testee.get(0)->flags, "[f4b]");
    TS_ASSERT_EQUALS(testee.get(0)->value, "v4");

    TS_ASSERT_EQUALS(testee.get(1)->name, "1a");
    TS_ASSERT_EQUALS(testee.get(1)->flags, "[f1a]");
    TS_ASSERT_EQUALS(testee.get(1)->value, "v1");

    TS_ASSERT_EQUALS(testee.get(2)->name, "3");
    TS_ASSERT_EQUALS(testee.get(2)->flags, "[f3]");
    TS_ASSERT_EQUALS(testee.get(2)->value, "v3");
}

