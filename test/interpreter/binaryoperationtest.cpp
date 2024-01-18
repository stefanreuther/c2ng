/**
  *  \file test/interpreter/binaryoperationtest.cpp
  *  \brief Test for interpreter::BinaryOperation
  */

#include "interpreter/binaryoperation.hpp"
#include "afl/test/testrunner.hpp"

/** Test interpreter::getBinaryName(). */
AFL_TEST("interpreter.BinaryOperation:getBinaryName", a)
{
    // Enum
    a.checkEqual("biAdd",    interpreter::getBinaryName(interpreter::biAdd),    String_t("add"));
    a.checkEqual("biMax",    interpreter::getBinaryName(interpreter::biMax),    String_t("max"));
    a.checkEqual("biMax_NC", interpreter::getBinaryName(interpreter::biMax_NC), String_t("maxNC"));

    // Integers
    a.checkEqual("0",        interpreter::getBinaryName(0), String_t("and"));
    a.checkEqual("255",      interpreter::getBinaryName(255), String_t("?"));
}
