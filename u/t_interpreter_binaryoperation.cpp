/**
  *  \file u/t_interpreter_binaryoperation.cpp
  *  \brief Test for interpreter::BinaryOperation
  */

#include "interpreter/binaryoperation.hpp"

#include "t_interpreter.hpp"

#define TS_ASSERT_CSTRLIT(a, b) TS_ASSERT_SAME_DATA(a, b, sizeof(b))

/** Test interpreter::getBinaryName(). */
void
TestInterpreterBinaryOperation::testName()
{
    // Enum
    TS_ASSERT_CSTRLIT(interpreter::getBinaryName(interpreter::biAdd), "add");
    TS_ASSERT_CSTRLIT(interpreter::getBinaryName(interpreter::biMax), "max");
    TS_ASSERT_CSTRLIT(interpreter::getBinaryName(interpreter::biMax_NC), "maxNC");

    // Integers
    TS_ASSERT_CSTRLIT(interpreter::getBinaryName(0), "and");
    TS_ASSERT_CSTRLIT(interpreter::getBinaryName(255), "?");
}

