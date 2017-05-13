/**
  *  \file u/t_interpreter_structuretypedata.cpp
  *  \brief Test for interpreter::StructureTypeData
  */

#include "interpreter/structuretypedata.hpp"

#include "t_interpreter.hpp"

/** Simple test. */
void
TestInterpreterStructureTypeData::testIt()
{
    interpreter::StructureTypeData sd;
    const interpreter::StructureTypeData& csd = sd;

    TS_ASSERT_EQUALS(&sd.names(), &csd.names());
}

