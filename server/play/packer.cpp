/**
  *  \file server/play/packer.cpp
  */

#include <stdexcept>
#include "server/play/packer.hpp"
#include "afl/string/format.hpp"

// /** Write value from script to JSON output. Automatically picks
//     integer or string output depending on property type. When the
//     property is not defined, it is not written to JSON output.
//     \param w JSON writer
//     \param con Object context
//     \param scriptName Name of property to obtain from context
//     \param jsonName Name of property in JSON output */
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

// /** Write value from script to JSON output. Automatically picks
//     integer or string output depending on property type. When the
//     value is not defined, it is not written to JSON output.
//     \param w JSON writer
//     \param value Value. Will be freed.
//     \param jsonout Name of property in JSON output */
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
