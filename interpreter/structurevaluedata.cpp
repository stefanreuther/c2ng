/**
  *  \file interpreter/structurevaluedata.cpp
  *  \brief Class interpreter::StructureValueData
  */

#include "interpreter/structurevaluedata.hpp"

// Constructor.
interpreter::StructureValueData::StructureValueData(StructureTypeData::Ref_t type)
    : m_type(type),
      m_data()
{
    // ex IntStructureValueData::IntStructureValueData
}

// Destructor.
interpreter::StructureValueData::~StructureValueData()
{ }

void
interpreter::StructureValueData::changeType(const StructureTypeData::Ref_t& type)
{
    m_type.reset(*type);
}
