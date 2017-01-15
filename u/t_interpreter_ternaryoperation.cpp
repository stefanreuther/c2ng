/**
  *  \file u/t_interpreter_ternaryoperation.cpp
  *  \brief Test for interpreter::TernaryOperation
  */

#include "interpreter/ternaryoperation.hpp"

#include "t_interpreter.hpp"

#define TS_ASSERT_CSTRLIT(a, b) TS_ASSERT_SAME_DATA(a, b, sizeof(b))

/** Test interpreter::getTernaryName(). */
void
TestInterpreterTernaryOperation::testName()
{
    // Enum
    TS_ASSERT_CSTRLIT(interpreter::getTernaryName(interpreter::teKeyAdd), "keyadd");

    // Integers
    TS_ASSERT_CSTRLIT(interpreter::getTernaryName(0), "keyadd");
    TS_ASSERT_CSTRLIT(interpreter::getTernaryName(255), "?");
}

