/**
  *  \file interpreter/arrayvalue.cpp
  *  \brief Class interpreter::ArrayValue
  */

#include "interpreter/arrayvalue.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/savecontext.hpp"

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
        return afl::data::Value::cloneOf(m_data->content[index]);
    } else {
        return 0;
    }
}

void
interpreter::ArrayValue::set(Arguments& args, afl::data::Value* value)
{
    // ex IntArray::set
    size_t index;
    if (m_data->computeIndex(args, index)) {
        m_data->content.set(index, value);
    } else {
        throw Error::typeError(Error::ExpectInteger);
    }
}

// CallableValue:
int32_t
interpreter::ArrayValue::getDimension(int32_t which) const
{
    // ex IntArray::getDimension
    // FIXME: range checks?
    if (which == 0) {
        return m_data->getNumDimensions();
    } else {
        return m_data->getDimension(which-1);
    }
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
    return "#<array>";
}

void
interpreter::ArrayValue::store(TagNode& out, afl::io::DataSink& /*aux*/, SaveContext& ctx) const
{
    // ex IntArray::store
    out.tag   = TagNode::Tag_Array;
    out.value = ctx.addArray(*m_data);
}

// Inquiry
afl::base::Ref<interpreter::ArrayData>
interpreter::ArrayValue::getData() const
{
    return m_data;
}
