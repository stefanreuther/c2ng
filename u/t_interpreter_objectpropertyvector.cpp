/**
  *  \file u/t_interpreter_objectpropertyvector.cpp
  *  \brief Test for interpreter::ObjectPropertyVector
  */

#include "interpreter/objectpropertyvector.hpp"

#include "t_interpreter.hpp"
#include "interpreter/values.hpp"

/** Simple test. */
void
TestInterpreterObjectPropertyVector::testIt()
{
    // Make empty vector
    interpreter::ObjectPropertyVector testee;
    TS_ASSERT(testee.get(-1) == 0);          // out of range
    TS_ASSERT(testee.get(0) == 0);           // out of range
    TS_ASSERT(testee.get(1) == 0);
    TS_ASSERT(testee.get(100) == 0);
    TS_ASSERT(testee.get(1, 0) == 0);
    TS_ASSERT(testee.get(100, 0) == 0);

    // Create - out of range
    afl::data::Segment* p = testee.create(0);
    TS_ASSERT(p == 0);

    p = testee.create(-1);
    TS_ASSERT(p == 0);

    // Create - ok
    p = testee.create(100);
    TS_ASSERT(p != 0);
    p->setNew(30, interpreter::makeStringValue("x"));

    p = testee.create(30);
    TS_ASSERT(p != 0);
    p->setNew(100, interpreter::makeStringValue("y"));

    p = testee.create(101);
    TS_ASSERT(p != 0);
    p->setNew(0, interpreter::makeStringValue("z"));

    // Get
    TS_ASSERT(testee.get(99) == 0);
    TS_ASSERT(testee.get(100) != 0);
    TS_ASSERT(testee.get(30) != 0);

    // Get values
    TS_ASSERT(testee.get(100, 0) == 0);
    TS_ASSERT(testee.get(100, 1) == 0);
    TS_ASSERT_EQUALS(interpreter::toString(testee.get(100, 30), false), "x");
    TS_ASSERT_EQUALS(interpreter::toString(testee.get(30, 100), false), "y");
    TS_ASSERT_EQUALS(interpreter::toString(testee.get(101, 0), false), "z");

    // Clear
    testee.clear();
    TS_ASSERT(testee.get(100) == 0);
    TS_ASSERT(testee.get(1, 0) == 0);
    TS_ASSERT(testee.get(100, 0) == 0);
}

