/**
  *  \file test/interpreter/ternaryoperationtest.cpp
  *  \brief Test for interpreter::TernaryOperation
  */

#include "interpreter/ternaryoperation.hpp"
#include "afl/test/testrunner.hpp"

/** Test interpreter::getTernaryName(). */
AFL_TEST("interpreter.TernaryOperation:getTernaryName", a)
{
    // Enum
    a.checkEqual("teKeyAdd", String_t(interpreter::getTernaryName(interpreter::teKeyAdd)), "keyadd");

    // Integers
    a.checkEqual("0", String_t(interpreter::getTernaryName(0)), "keyadd");
    a.checkEqual("255", String_t(interpreter::getTernaryName(255)), "?");
}
