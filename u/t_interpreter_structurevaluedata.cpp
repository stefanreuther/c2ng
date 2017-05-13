/**
  *  \file u/t_interpreter_structurevaluedata.cpp
  *  \brief Test for interpreter::StructureValueData
  */

#include "interpreter/structurevaluedata.hpp"

#include "t_interpreter.hpp"

/** Simple test.
    This is a structure, so there isn't much to test for now. */
void
TestInterpreterStructureValueData::testIt()
{
    interpreter::StructureValueData testee(*new interpreter::StructureTypeData());
}

