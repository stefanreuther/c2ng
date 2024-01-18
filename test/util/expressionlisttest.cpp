/**
  *  \file test/util/expressionlisttest.cpp
  *  \brief Test for util::ExpressionList
  */

#include "util/expressionlist.hpp"
#include "afl/test/testrunner.hpp"

/** Test most access operations. */
AFL_TEST("util.ExpressionList:basics", a)
{
    util::ExpressionList testee;
    size_t pos = 0;

    // Verify initial state
    a.check     ("01. empty",              testee.empty());
    a.checkEqual("02. size",               testee.size(), 0U);
    a.checkNull ("03. get",                testee.get(0));
    a.check     ("04. findIndexForValue", !testee.findIndexForValue("v").get(pos));

    // Add some values
    testee.pushBackNew(new util::ExpressionList::Item("n1", "[f1]", "v1"));
    testee.pushBackNew(new util::ExpressionList::Item("n2", "[f2]", "v2"));
    testee.pushBackNew(new util::ExpressionList::Item("n", "[f]", "v"));
    testee.pushBackNew(new util::ExpressionList::Item("n3", "[f3]", "v3"));

    a.check       ("11. empty", !testee.empty());
    a.checkEqual  ("12. size",   testee.size(), 4U);
    a.checkNonNull("13. get",    testee.get(0));
    a.checkNonNull("14. get",    testee.get(3));
    a.checkNull   ("15. get",    testee.get(4));

    a.checkEqual("21. name",  testee.get(0)->name, "n1");
    a.checkEqual("22. flags", testee.get(0)->flags, "[f1]");
    a.checkEqual("23. value", testee.get(0)->value, "v1");

    a.check("31. findIndexForValue", testee.findIndexForValue("v").get(pos));
    a.checkEqual("32. pos", pos, 2U);

    // Move to front
    testee.moveToFront(2);
    a.checkEqual("41. size", testee.size(), 4U);
    a.checkEqual("42. name", testee.get(0)->name, "n");
    a.check     ("43. findIndexForValue", testee.findIndexForValue("v").get(pos));
    a.checkEqual("44. pos", pos, 0U);

    // Clear
    testee.clear();
    a.check     ("51. empty",              testee.empty());
    a.checkEqual("52. size",               testee.size(), 0U);
    a.checkNull ("53. get",                testee.get(0));
    a.check     ("54. findIndexForValue", !testee.findIndexForValue("v").get(pos));
}

/** Test LRU behaviour. */
AFL_TEST("util.ExpressionList:lru", a)
{
    util::ExpressionList testee;
    testee.pushFrontNew(new util::ExpressionList::Item("1", "[f1]", "v1"), 3);      // v1
    testee.pushFrontNew(new util::ExpressionList::Item("2", "[f2]", "v2"), 3);      // v2:v1
    testee.pushFrontNew(new util::ExpressionList::Item("3", "[f3]", "v3"), 3);      // v3:v2:v1
    testee.pushFrontNew(new util::ExpressionList::Item("1a", "[f1a]", "v1"), 3);    // v1a:v3:v2
    testee.pushFrontNew(new util::ExpressionList::Item("4", "[f4]", "v4"), 3);      // v4:v1a:v3
    testee.pushFrontNew(new util::ExpressionList::Item("4b", "[f4b]", "v4"), 3);    // v4b:v1a:v3

    a.checkEqual("01. size", testee.size(), 3U);

    a.checkEqual("11", testee.get(0)->name, "4b");
    a.checkEqual("12", testee.get(0)->flags, "[f4b]");
    a.checkEqual("13", testee.get(0)->value, "v4");

    a.checkEqual("21", testee.get(1)->name, "1a");
    a.checkEqual("22", testee.get(1)->flags, "[f1a]");
    a.checkEqual("23", testee.get(1)->value, "v1");

    a.checkEqual("31", testee.get(2)->name, "3");
    a.checkEqual("32", testee.get(2)->flags, "[f3]");
    a.checkEqual("33", testee.get(2)->value, "v3");
}
