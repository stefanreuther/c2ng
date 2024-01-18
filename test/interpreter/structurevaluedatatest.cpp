/**
  *  \file test/interpreter/structurevaluedatatest.cpp
  *  \brief Test for interpreter::StructureValueData
  */

#include "interpreter/structurevaluedata.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test.
    This is a structure, so there isn't much to test for now. */
AFL_TEST_NOARG("interpreter.StructureValueData")
{
    interpreter::StructureValueData testee(*new interpreter::StructureTypeData());
}
