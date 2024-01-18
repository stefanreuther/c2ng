/**
  *  \file test/util/stringlisttest.cpp
  *  \brief Test for util::StringList
  */

#include "util/stringlist.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("util.StringList:basics", a)
{
    util::StringList testee;
    int32_t i;
    String_t s;

    // Verify empty
    a.checkEqual("01", testee.size(), 0U);
    a.check("02", testee.empty());
    a.check("03", !testee.get(0, i, s));
    a.check("04", !testee.get(size_t(-1), i, s));
    a.check("05", !testee.get(1000000, i, s));

    // Populate
    testee.add(23, "hi");
    testee.add(42, "ho");
    a.checkEqual("11", testee.size(), 2U);
    a.check("12", !testee.empty());

    // Verify populated
    a.check("21", testee.get(0, i, s));
    a.checkEqual("22", i, 23);
    a.checkEqual("23", s, "hi");

    a.check("31", testee.get(1, i, s));
    a.checkEqual("32", i, 42);
    a.checkEqual("33", s, "ho");

    a.check("41", !testee.get(size_t(-1), i, s));
    a.check("42", !testee.get(1000000, i, s));

    // Verify find
    size_t n;
    a.check("51", !testee.find(0).get(n));
    a.check("52", !testee.find(1).get(n));
    a.check("53", testee.find(42).get(n));
    a.checkEqual("54", n, 1U);

    // Add some more
    testee.add(3, "x");         // 2
    testee.add(1, "y");         // 3
    testee.add(4, "z");         // 4
    testee.add(1, "a");         // 5
    testee.add(5, "b");         // 6
    a.check("61", testee.find(1).get(n));
    a.checkEqual("62", n, 3U);    // first instance of 1
}

/** Test sort. */
AFL_TEST("util.StringList:sort", a)
{
    util::StringList testee;
    testee.add(1, "foo");
    testee.add(2, "bar");
    testee.add(3, "baz");
    testee.add(4, "qux");
    testee.sortAlphabetically();

    a.checkEqual("01", testee.size(), 4U);

    int32_t id;
    String_t value;
    a.check("11", testee.get(0, id, value));
    a.checkEqual("12", id, 2);
    a.checkEqual("13", value, "bar");

    a.check("21", testee.get(1, id, value));
    a.checkEqual("22", id, 3);
    a.checkEqual("23", value, "baz");

    a.check("31", testee.get(2, id, value));
    a.checkEqual("32", id, 1);
    a.checkEqual("33", value, "foo");

    a.check("41", testee.get(3, id, value));
    a.checkEqual("42", id, 4);
    a.checkEqual("43", value, "qux");
}

/** Test copy, swap, clear. */
AFL_TEST("util.StringList:copy", a)
{
    util::StringList sa;
    sa.add(1, "foo");
    sa.add(2, "bar");
    a.checkEqual("01", sa.size(), 2U);

    util::StringList sb(sa);
    a.checkEqual("11", sb.size(), 2U);

    util::StringList sc;
    a.checkEqual("21", sc.size(), 0U);

    sa.swap(sc);
    a.checkEqual("31", sc.size(), 2U);
    a.checkEqual("32", sa.size(), 0U);

    sa = sc;
    a.checkEqual("41", sc.size(), 2U);
    a.checkEqual("42", sa.size(), 2U);

    sa.clear();
    a.checkEqual("51", sa.size(), 0U);
}
