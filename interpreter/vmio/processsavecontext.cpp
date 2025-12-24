/**
  *  \file interpreter/vmio/processsavecontext.cpp
  *  \brief Class interpreter::vmio::ProcessSaveContext
  */

#include "interpreter/vmio/processsavecontext.hpp"

interpreter::vmio::ProcessSaveContext::ProcessSaveContext(SaveContext& parent, const Process& process)
    : m_parent(parent),
      m_process(process)
{ }

uint32_t
interpreter::vmio::ProcessSaveContext::addBCO(const BCORef_t& bco)
{
    return m_parent.addBCO(bco);
}

uint32_t
interpreter::vmio::ProcessSaveContext::addHash(const afl::data::Hash::Ref_t& hash)
{
    return m_parent.addHash(hash);
}

uint32_t
interpreter::vmio::ProcessSaveContext::addArray(const ArrayData::Ref_t& array)
{
    return m_parent.addArray(array);
}

uint32_t
interpreter::vmio::ProcessSaveContext::addStructureType(const StructureTypeData::Ref_t& type)
{
    return m_parent.addStructureType(type);
}

uint32_t
interpreter::vmio::ProcessSaveContext::addStructureValue(const StructureValueData::Ref_t& value)
{
    return m_parent.addStructureValue(value);
}

bool
interpreter::vmio::ProcessSaveContext::isCurrentProcess(const Process* p)
{
    return p == &m_process;
}
