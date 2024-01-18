/**
  *  \file test/util/vectortest.cpp
  *  \brief Test for util::Vector
  */

#include "util/vector.hpp"
#include "afl/test/testrunner.hpp"

/** Test 0-based vector. */
AFL_TEST("util.Vector:0-based", a)
{
    // Create empty vector and verify it
    util::Vector<char,int> testee;
    a.checkNull("01. at",     testee.at(0));
    a.checkEqual("02. size",  testee.size(), 0);
    a.checkEqual("03. get",   testee.get(0), '\0');
    a.checkEqual("04. get",   testee.get(99), '\0');
    a.checkEqual("05. empty", testee.empty(), true);

    // Set one element and verify
    testee.set(2, 'x');
    a.checkEqual("11. size",  testee.size(), 3);
    a.checkEqual("12. get",   testee.get(0), '\0');
    a.checkEqual("13. get",   testee.get(2), 'x');
    a.checkEqual("14. get",   testee.get(99), '\0');
    a.checkEqual("15. empty", testee.empty(), false);

    a.checkNonNull("21. at", testee.at(0));
    a.checkNonNull("22. at", testee.at(1));
    a.checkNonNull("23. at", testee.at(2));
    a.checkNull("24. at",    testee.at(3));
    a.checkEqual("25. at",  *testee.at(2), 'x');

    // Clear and verify
    testee.clear();
    a.checkNull("31. at",     testee.at(0));
    a.checkEqual("32. size",  testee.size(), 0);
    a.checkEqual("33. get",   testee.get(0), '\0');
    a.checkEqual("34. get",   testee.get(99), '\0');
    a.checkEqual("35. empty", testee.empty(), true);
}

/** Test 1-based vector. */
AFL_TEST("util.Vector:1-based", a)
{
    // Create empty vector and verify it
    util::Vector<char,int> testee(1);
    a.checkNull("01. at",     testee.at(0));
    a.checkEqual("02. size",  testee.size(), 1);
    a.checkEqual("03. get",   testee.get(0), '\0');
    a.checkEqual("04. get",   testee.get(99), '\0');
    a.checkEqual("05. empty", testee.empty(), true);

    // Set one element and verify
    testee.set(2, 'x');
    a.checkEqual("11. size",  testee.size(), 3);
    a.checkEqual("12. get",   testee.get(0), '\0');
    a.checkEqual("13. get",   testee.get(2), 'x');
    a.checkEqual("14. get",   testee.get(99), '\0');
    a.checkEqual("15. empty", testee.empty(), false);

    a.checkNull("21. at",    testee.at(0));
    a.checkNonNull("22. at", testee.at(1));
    a.checkNonNull("23. at", testee.at(2));
    a.checkNull("24. at",    testee.at(3));
    a.checkEqual("25. at",  *testee.at(2), 'x');
}
