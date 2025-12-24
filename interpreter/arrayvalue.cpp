/**
  *  \file interpreter/arrayvalue.cpp
  *  \brief Class interpreter::ArrayValue
  */

#include "interpreter/arrayvalue.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/savecontext.hpp"
#include "interpreter/values.hpp"

namespace {
    // Arbitrary length limit:
    const size_t TOSTRING_MAX = 200;
}

// Constructor.
interpreter::ArrayValue::ArrayValue(afl::base::Ref<ArrayData> data)
    : m_data(data)
{ }

// IndexableValue:
afl::data::Value*
interpreter::ArrayValue::get(Arguments& args)
{
    // ex IntArray::get
    size_t index;
    if (m_data->computeIndex(args, index)) {
        return afl::data::Value::cloneOf(m_data->content()[index]);
    } else {
        return 0;
    }
}

void
interpreter::ArrayValue::set(Arguments& args, const afl::data::Value* value)
{
    // ex IntArray::set
    size_t index;
    if (m_data->computeIndex(args, index)) {
        m_data->content().set(index, value);
    } else {
        throw Error::typeError(Error::ExpectInteger);
    }
}

// CallableValue:
size_t
interpreter::ArrayValue::getDimension(size_t which) const
{
    // ex IntArray::getDimension
    size_t value;
    if (which == 0) {
        value = m_data->getNumDimensions();
    } else {
        value = m_data->getDimension(which-1);
    }
    return value;
}

interpreter::Context*
interpreter::ArrayValue::makeFirstContext()
{
    // ex IntArray::makeFirstContext()
    return rejectFirstContext();
}

// BaseValue:
interpreter::ArrayValue*
interpreter::ArrayValue::clone() const
{
    // ex IntArray::clone
    return new ArrayValue(m_data);
}

String_t
interpreter::ArrayValue::toString(bool /*readable*/) const
{
    // ex IntArray::toString
    if (m_data->getNumDimensions() == 1) {
        String_t result = "Array(";
        bool ok = true;
        for (size_t i = 0, n = m_data->getDimension(0); i < n; ++i) {
            if (i != 0) {
                result += ",";
            }
            if (dynamic_cast<ArrayValue*>(m_data->content()[i]) != 0) {
                // Quick and dirty detection of recursive data structure
                ok = false;
                break;
            }
            result += interpreter::toString(m_data->content()[i], true);
            if (result.size() > TOSTRING_MAX) {
                ok = false;
                break;
            }
        }
        if (ok) {
            result += ")";
            return result;
        }
    }
    return "#<array>";
}

void
interpreter::ArrayValue::store(TagNode& out, afl::io::DataSink& /*aux*/, SaveContext& ctx) const
{
    // ex IntArray::store
    out.tag   = TagNode::Tag_Array;
    out.value = ctx.addArray(m_data);
}

// Inquiry
afl::base::Ref<interpreter::ArrayData>
interpreter::ArrayValue::getData() const
{
    return m_data;
}
