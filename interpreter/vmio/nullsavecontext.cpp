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
interpreter::vmio::NullSaveContext::addBCO(BytecodeObject& /*bco*/)
{
    return fail();
}

uint32_t
interpreter::vmio::NullSaveContext::addHash(HashData& /*hash*/)
{
    return fail();
}

uint32_t
interpreter::vmio::NullSaveContext::addArray(ArrayData& /*array*/)
{
    return fail();
}

uint32_t
interpreter::vmio::NullSaveContext::addStructureType(StructureTypeData& /*type*/)
{
    return fail();
}

uint32_t
interpreter::vmio::NullSaveContext::addStructureValue(StructureValueData& /*value*/)
{
    return fail();
}

bool
interpreter::vmio::NullSaveContext::isCurrentProcess(Process* /*p*/)
{
    return fail();
}
