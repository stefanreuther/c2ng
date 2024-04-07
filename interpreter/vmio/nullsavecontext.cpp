/**
  *  \file interpreter/vmio/nullsavecontext.cpp
  *  \brief Class interpreter::vmio::NullSaveContext
  */

#include "interpreter/vmio/nullsavecontext.hpp"
#include "interpreter/error.hpp"

namespace {
    int fail()
    {
        throw interpreter::Error::notSerializable();
    }
}

uint32_t
interpreter::vmio::NullSaveContext::addBCO(const BytecodeObject& /*bco*/)
{
    return fail();
}

uint32_t
interpreter::vmio::NullSaveContext::addHash(const afl::data::Hash& /*hash*/)
{
    return fail();
}

uint32_t
interpreter::vmio::NullSaveContext::addArray(const ArrayData& /*array*/)
{
    return fail();
}

uint32_t
interpreter::vmio::NullSaveContext::addStructureType(const StructureTypeData& /*type*/)
{
    return fail();
}

uint32_t
interpreter::vmio::NullSaveContext::addStructureValue(const StructureValueData& /*value*/)
{
    return fail();
}

bool
interpreter::vmio::NullSaveContext::isCurrentProcess(const Process* /*p*/)
{
    return false;
}
