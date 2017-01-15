/**
  *  \file interpreter/vmio/processsavecontext.cpp
  */

#include "interpreter/vmio/processsavecontext.hpp"

interpreter::vmio::ProcessSaveContext::ProcessSaveContext(SaveContext& parent, Process& process)
    : m_parent(parent),
      m_process(process)
{ }

uint32_t
interpreter::vmio::ProcessSaveContext::addBCO(BytecodeObject& bco)
{
    return m_parent.addBCO(bco);
}

uint32_t
interpreter::vmio::ProcessSaveContext::addHash(HashData& hash)
{
    return m_parent.addHash(hash);
}

uint32_t
interpreter::vmio::ProcessSaveContext::addArray(ArrayData& array)
{
    return m_parent.addArray(array);
}

uint32_t
interpreter::vmio::ProcessSaveContext::addStructureType(StructureTypeData& type)
{
    return m_parent.addStructureType(type);
}

uint32_t
interpreter::vmio::ProcessSaveContext::addStructureValue(StructureValueData& value)
{
    return m_parent.addStructureValue(value);
}

bool
interpreter::vmio::ProcessSaveContext::isCurrentProcess(Process* p)
{
    return p == &m_process;
}
