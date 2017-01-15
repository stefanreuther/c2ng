/**
  *  \file u/t_interpreter_unaryoperation.cpp
  *  \brief Test for interpreter::UnaryOperation
  */

#include "interpreter/unaryoperation.hpp"

#include "t_interpreter.hpp"

#define TS_ASSERT_CSTRLIT(a, b) TS_ASSERT_SAME_DATA(a, b, sizeof(b))

/** Test interpreter::getUnaryName(). */
void
TestInterpreterUnaryOperation::testName()
{
    // Enum
    TS_ASSERT_CSTRLIT(interpreter::getUnaryName(interpreter::unInc), "inc");
    TS_ASSERT_CSTRLIT(interpreter::getUnaryName(interpreter::unIsEmpty), "isempty");

    // Integers
    TS_ASSERT_CSTRLIT(interpreter::getUnaryName(0), "not");
    TS_ASSERT_CSTRLIT(interpreter::getUnaryName(255), "?");
}

