/**
  *  \file test/interpreter/structuretypedatatest.cpp
  *  \brief Test for interpreter::StructureTypeData
  */

#include "interpreter/structuretypedata.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("interpreter.StructureTypeData", a)
{
    interpreter::StructureTypeData sd;
    const interpreter::StructureTypeData& csd = sd;

    a.checkEqual("01. names", &sd.names(), &csd.names());
}
