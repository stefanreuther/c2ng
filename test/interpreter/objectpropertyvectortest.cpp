/**
  *  \file test/interpreter/objectpropertyvectortest.cpp
  *  \brief Test for interpreter::ObjectPropertyVector
  */

#include "interpreter/objectpropertyvector.hpp"

#include "afl/test/testrunner.hpp"
#include "interpreter/values.hpp"

/** Simple test. */
AFL_TEST("interpreter.ObjectPropertyVector:basics", a)
{
    // Make empty vector
    interpreter::ObjectPropertyVector testee;
    a.checkNull("01. get", testee.get(-1));          // out of range
    a.checkNull("02. get", testee.get(0));           // out of range
    a.checkNull("03. get", testee.get(1));
    a.checkNull("04. get", testee.get(100));
    a.checkNull("05. get", testee.get(1, 0));
    a.checkNull("06. get", testee.get(100, 0));

    // Create - out of range
    afl::data::Segment* p = testee.create(0);
    a.checkNull("11. create", p);

    p = testee.create(-1);
    a.checkNull("21. create", p);

    // Create - ok
    p = testee.create(100);
    a.checkNonNull("31. create", p);
    p->setNew(30, interpreter::makeStringValue("x"));

    p = testee.create(30);
    a.checkNonNull("41. create", p);
    p->setNew(100, interpreter::makeStringValue("y"));

    p = testee.create(101);
    a.checkNonNull("51. create", p);
    p->setNew(0, interpreter::makeStringValue("z"));

    // Get
    a.checkNull   ("61. get", testee.get(99));
    a.checkNonNull("62. get", testee.get(100));
    a.checkNonNull("63. get", testee.get(30));

    // Get values
    a.checkNull ("71. get", testee.get(100, 0));
    a.checkNull ("72. get", testee.get(100, 1));
    a.checkEqual("73. get", interpreter::toString(testee.get(100, 30), false), "x");
    a.checkEqual("74. get", interpreter::toString(testee.get(30, 100), false), "y");
    a.checkEqual("75. get", interpreter::toString(testee.get(101, 0), false), "z");

    // Clear
    testee.clear();
    a.checkNull("81. get", testee.get(100));
    a.checkNull("82. get", testee.get(1, 0));
    a.checkNull("83. get", testee.get(100, 0));
}
