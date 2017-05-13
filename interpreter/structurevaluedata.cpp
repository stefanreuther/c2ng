/**
  *  \file interpreter/structurevaluedata.cpp
  *  \brief Class interpreter::StructureValueData
  */

#include "interpreter/structurevaluedata.hpp"

// Constructor.
interpreter::StructureValueData::StructureValueData(StructureTypeData::Ref_t type)
    : type(type),
      data()
{
    // ex IntStructureValueData::IntStructureValueData
}

// Destructor.
interpreter::StructureValueData::~StructureValueData()
{ }
