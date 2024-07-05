/**
  *  \file server/play/packer.cpp
  *  \brief Interface server::play::Packer
  */

#include <stdexcept>
#include "server/play/packer.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/string/format.hpp"
#include "game/interface/referencecontext.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/arrayvalue.hpp"

using afl::data::Vector;
using afl::data::VectorValue;
using game::interface::ReferenceContext;
using interpreter::Arguments;
using interpreter::ArrayData;
using interpreter::ArrayValue;
using server::play::Packer;

namespace {
    Vector::Ref_t packArray(const ArrayData& ad, size_t dim, Arguments& reader)
    {
        Vector::Ref_t vec = Vector::create();
        const size_t n = ad.getDimension(dim);
        if (dim+1 >= ad.getNumDimensions()) {
            // Final dimension
            for (size_t i = 0; i < n; ++i) {
                vec->pushBackNew(Packer::flattenNew(afl::data::Value::cloneOf(reader.getNext())));
            }
        } else {
            // Intermediate dimension
            for (size_t i = 0; i < n; ++i) {
                vec->pushBackNew(new VectorValue(packArray(ad, dim+1, reader)));
            }
        }
        return vec;
    }
}

// Fetch value from Context and add to Hash.
void
server::play::Packer::addValue(afl::data::Hash& hv, interpreter::Context& ctx, const char* scriptName, const char* jsonName)
{
    // ex server/objout.cc:writeValue
    interpreter::Context::PropertyIndex_t index;
    interpreter::Context::PropertyAccessor* pContext = ctx.lookup(scriptName, index);
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
    if (Value_t* v = flattenNew(value)) {
        hv.setNew(jsonName, v);
    }
}

// Flatten a value for serialisation as JSON.
server::Value_t*
server::play::Packer::flattenNew(Value_t* value)
{
    std::auto_ptr<Value_t> p(value);
    if (const ArrayValue* av = dynamic_cast<const ArrayValue*>(p.get())) {
        const ArrayData& ad = *av->getData();
        Arguments reader(ad.content(), 0, ad.content().size());
        p.reset(new VectorValue(packArray(ad, 0, reader)));
    } else if (const ReferenceContext* ref = dynamic_cast<const ReferenceContext*>(p.get())) {
        Vector::Ref_t vec = Vector::create();
        game::map::Point pt;
        if (ref->getReference().getPosition().get(pt)) {
            vec->pushBackString("location");
            vec->pushBackInteger(pt.getX());
            vec->pushBackInteger(pt.getY());
        } else if (const char* name = game::interface::getReferenceTypeName(ref->getReference().getType())) {
            vec->pushBackString(name);
            vec->pushBackInteger(ref->getReference().getId());
        } else {
            // Null reference; should not happen
        }
        p.reset(new VectorValue(vec));
    } else {
        // Leave as-is
    }
    return p.release();
}
