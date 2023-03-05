/**
  *  \file interpreter/structurevalue.cpp
  *  \brief Class interpreter::StructureValue
  */

#include "interpreter/structurevalue.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/savecontext.hpp"

// Constructor.
interpreter::StructureValue::StructureValue(StructureValueData::Ref_t value)
    : m_value(value)
{
    // ex IntStructureValue::IntStructureValue
}

// Destructor.
interpreter::StructureValue::~StructureValue()
{ }

// BaseValue:
String_t
interpreter::StructureValue::toString(bool /*readable*/) const
{
    // ex IntStructureValue::toString
    return "#<struct>";
}

void
interpreter::StructureValue::store(TagNode& out, afl::io::DataSink& /*aux*/, SaveContext& ctx) const
{
    // ex IntStructureValue::store
    out.tag   = TagNode::Tag_Struct;
    out.value = ctx.addStructureValue(*m_value);
}

// Value:
interpreter::StructureValue*
interpreter::StructureValue::clone() const
{
    // ex IntStructureValue::clone
    return new StructureValue(m_value);
}

// Context:
interpreter::Context::PropertyAccessor*
interpreter::StructureValue::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntStructureValue::lookup
    afl::data::NameMap::Index_t index = m_value->type().names().getIndexByName(name);
    if (index != afl::data::NameMap::nil) {
        result = index;
        return this;
    } else {
        return 0;
    }
}

void
interpreter::StructureValue::set(PropertyIndex_t index, const afl::data::Value* value)
{
    // ex IntStructureValue::set
    m_value->data().set(index, value);
}

afl::data::Value*
interpreter::StructureValue::get(PropertyIndex_t index)
{
    // ex IntStructureValue::get
    return afl::data::Value::cloneOf(m_value->data()[index]);
}

afl::base::Deletable*
interpreter::StructureValue::getObject()
{
    // ex IntStructureValue::getObject
    return 0;
}

void
interpreter::StructureValue::enumProperties(PropertyAcceptor& acceptor) const
{
    // ex IntStructureValue::enumProperties
    acceptor.enumNames(m_value->type().names());
}
