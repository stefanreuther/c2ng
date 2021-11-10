/**
  *  \file interpreter/simpleindexablevalue.cpp
  */

#include "interpreter/simpleindexablevalue.hpp"
#include "interpreter/error.hpp"

interpreter::SimpleIndexableValue::SimpleIndexableValue(World& world, Get_t* get, Dim_t* dim, Make_t* make)
    : m_world(world),
      m_get(get),
      m_dim(dim),
      m_make(make)
{ }

interpreter::SimpleIndexableValue::~SimpleIndexableValue()
{ }

// IndexableValue:
afl::data::Value*
interpreter::SimpleIndexableValue::get(Arguments& args)
{
    // ex IntSimpleIndexableValue::get
    if (m_get) {
        return m_get(m_world, args);
    } else {
        throw Error::typeError(Error::ExpectIndexable);
    }
}

void
interpreter::SimpleIndexableValue::set(Arguments& /*args*/, afl::data::Value* /*value*/)
{
    // ex IntSimpleIndexableValue::set
    throw Error::notAssignable();
}

int32_t
interpreter::SimpleIndexableValue::getDimension(int32_t which) const
{
    // ex IntSimpleIndexableValue::getDimension
    if (m_dim) {
        return m_dim(m_world, which);
    } else {
        return 0;
    }
}

// CallableValue:
interpreter::Context*
interpreter::SimpleIndexableValue::makeFirstContext()
{
    // ex IntSimpleIndexableValue::makeFirstContext
    if (m_make) {
        return m_make(m_world);
    } else {
        throw Error::typeError(Error::ExpectIterable);
    }
}

interpreter::SimpleIndexableValue*
interpreter::SimpleIndexableValue::clone() const
{
    // ex IntSimpleIndexableValue::clone
    return new SimpleIndexableValue(m_world, m_get, m_dim, m_make);
}

// BaseValue:
String_t
interpreter::SimpleIndexableValue::toString(bool /*readable*/) const
{
    return "#<function>";
}

void
interpreter::SimpleIndexableValue::store(TagNode& /*out*/, afl::io::DataSink& /*aux*/, SaveContext& /*ctx*/) const
{
    throw Error::notSerializable();
}
