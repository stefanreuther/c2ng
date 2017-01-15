/**
  *  \file interpreter/structurevalue.cpp
  *
  *  A structure type has a fixed name/value-slot mapping for all objects
  *  of its type (unlike a hash, which has a possibly-different mapping
  *  for each object). We therefore have two objects, the structure type
  *  (IntStructureTypeData) for the mapping, and the actual instance value
  *  (IntStructureValueData). As usual, we have two C++ types for each,
  *  the actual "heavy-weight" data object, and the "light-weight"
  *  interpreter object.
  */

#include "interpreter/structurevalue.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/savecontext.hpp"

interpreter::StructureValueData::StructureValueData(afl::base::Ref<StructureTypeData> type)
    : type(type),
      data()
{
    // ex IntStructureValueData::IntStructureValueData
}

interpreter::StructureValueData::~StructureValueData()
{ }



interpreter::StructureValue::StructureValue(afl::base::Ref<StructureValueData> value)
    : m_value(value)
{
    // ex IntStructureValue::IntStructureValue
}

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
interpreter::StructureValue::store(TagNode& out, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, SaveContext& ctx) const
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
interpreter::StructureValue*
interpreter::StructureValue::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntStructureValue::lookup
    afl::data::NameMap::Index_t index = m_value->type->names.getIndexByName(name);
    if (index != afl::data::NameMap::nil) {
        result = index;
        return this;
    } else {
        return 0;
    }
}

void
interpreter::StructureValue::set(PropertyIndex_t index, afl::data::Value* value)
{
    // ex IntStructureValue::set
    m_value->data.set(index, value);
}

afl::data::Value*
interpreter::StructureValue::get(PropertyIndex_t index)
{
    // ex IntStructureValue::get
    return afl::data::Value::cloneOf(m_value->data[index]);
}

game::map::Object*
interpreter::StructureValue::getObject()
{
    // ex IntStructureValue::getObject
    return 0;
}

void
interpreter::StructureValue::enumProperties(PropertyAcceptor& acceptor)
{
    // ex IntStructureValue::enumProperties
    acceptor.enumNames(m_value->type->names);
}
