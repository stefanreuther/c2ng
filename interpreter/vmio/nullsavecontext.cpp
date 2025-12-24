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
interpreter::vmio::NullSaveContext::addBCO(const BCORef_t& /*bco*/)
{
    return fail();
}

uint32_t
interpreter::vmio::NullSaveContext::addHash(const afl::data::Hash::Ref_t& /*hash*/)
{
    return fail();
}

uint32_t
interpreter::vmio::NullSaveContext::addArray(const ArrayData::Ref_t& /*array*/)
{
    return fail();
}

uint32_t
interpreter::vmio::NullSaveContext::addStructureType(const StructureTypeData::Ref_t& /*type*/)
{
    return fail();
}

uint32_t
interpreter::vmio::NullSaveContext::addStructureValue(const StructureValueData::Ref_t& /*value*/)
{
    return fail();
}

bool
interpreter::vmio::NullSaveContext::isCurrentProcess(const Process* /*p*/)
{
    return false;
}
