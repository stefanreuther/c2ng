/**
  *  \file interpreter/vmio/nullloadcontext.cpp
  *  \brief Class interpreter::vmio::NullLoadContext
  */

#include "interpreter/vmio/nullloadcontext.hpp"

interpreter::vmio::NullLoadContext::~NullLoadContext()
{ }

afl::data::Value*
interpreter::vmio::NullLoadContext::loadBCO(uint32_t /*id*/)
{
    return 0;
}

afl::data::Value*
interpreter::vmio::NullLoadContext::loadArray(uint32_t /*id*/)
{
    return 0;
}

afl::data::Value*
interpreter::vmio::NullLoadContext::loadHash(uint32_t /*id*/)
{
    return 0;
}

afl::data::Value*
interpreter::vmio::NullLoadContext::loadStructureValue(uint32_t /*id*/)
{
    return 0;
}

afl::data::Value*
interpreter::vmio::NullLoadContext::loadStructureType(uint32_t /*id*/)
{
    return 0;
}

interpreter::Context*
interpreter::vmio::NullLoadContext::loadContext(const TagNode& /*tag*/, afl::io::Stream& /*aux*/)
{
    return 0;
}

interpreter::Process*
interpreter::vmio::NullLoadContext::createProcess()
{
    return 0;
}

void
interpreter::vmio::NullLoadContext::finishProcess(Process& /*proc*/)
{ }
