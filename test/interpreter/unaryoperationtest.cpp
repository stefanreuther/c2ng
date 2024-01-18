/**
  *  \file test/interpreter/unaryoperationtest.cpp
  *  \brief Test for interpreter::UnaryOperation
  */

#include "interpreter/unaryoperation.hpp"
#include "afl/test/testrunner.hpp"

/** Test interpreter::getUnaryName(). */
AFL_TEST("interpreter.UnaryOperation:getUnaryName", a)
{
    // Enum
    a.checkEqual("unInc",     String_t(interpreter::getUnaryName(interpreter::unInc)), "inc");
    a.checkEqual("unIsEmpty", String_t(interpreter::getUnaryName(interpreter::unIsEmpty)), "isempty");

    // Integers
    a.checkEqual("0",   String_t(interpreter::getUnaryName(0)), "not");
    a.checkEqual("255", String_t(interpreter::getUnaryName(255)), "?");
}
