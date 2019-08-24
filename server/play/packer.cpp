/**
  *  \file server/play/packer.cpp
  *  \brief Interface server::play::Packer
  */

#include <stdexcept>
#include "server/play/packer.hpp"
#include "afl/string/format.hpp"

// Fetch value from Context and add to Hash.
void
server::play::Packer::addValue(afl::data::Hash& hv, interpreter::Context& ctx, const char* scriptName, const char* jsonName)
{
    // ex server/objout.cc:writeValue
    interpreter::Context::PropertyIndex_t index;
    interpreter::Context* pContext = ctx.lookup(scriptName, index);
    if (pContext == 0) {
        // Name not found. Make this a hard error. (In PCC2, only a soft error.)
        throw std::runtime_error(afl::string::Format("Unable to resolve name \"%s\"", scriptName));
    }

    addValueNew(hv, pContext->get(index), jsonName);
}

// Add new value to Hash.
void
server::play::Packer::addValueNew(afl::data::Hash& hv, Value_t* value, const char* jsonName)
{
    // ex server/objout.cc:writeValue
    // This is totally different than the PCC2 version.
    // We support all types.
    if (value != 0) {
        hv.setNew(jsonName, value);
    }
}
