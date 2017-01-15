/**
  *  \file game/interface/simplefunction.cpp
  */

#include "game/interface/simplefunction.hpp"
#include "interpreter/error.hpp"

game::interface::SimpleFunction::SimpleFunction(Session& session, Function_t func)
    : m_session(session),
      m_function(func)
{
    // ex IntSimpleIndexableValue::IntSimpleIndexableValue, sort-of
}

game::interface::SimpleFunction::~SimpleFunction()
{ }

// IndexableValue:
afl::data::Value*
game::interface::SimpleFunction::get(interpreter::Arguments& args)
{
    // ex IntSimpleIndexableValue::get
    if (m_function != 0) {
        return m_function(m_session, args);
    } else {
        throw interpreter::Error::typeError(interpreter::Error::ExpectIndexable);
    }
}

void
game::interface::SimpleFunction::set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
{
    // ex IntSimpleIndexableValue::set
    throw interpreter::Error::notAssignable();
}


// CallableValue:
int32_t
game::interface::SimpleFunction::getDimension(int32_t /*which*/) const
{
    // ex IntSimpleIndexableValue::getDimension, sort-of
    return 0;
}

interpreter::Context*
game::interface::SimpleFunction::makeFirstContext()
{
    // ex IntSimpleIndexableValue::makeFirstContext, sort-of
    throw interpreter::Error::typeError(interpreter::Error::ExpectIterable);
}

game::interface::SimpleFunction*
game::interface::SimpleFunction::clone() const
{
    return new SimpleFunction(m_session, m_function);
}

// BaseValue:
String_t
game::interface::SimpleFunction::toString(bool /*readable*/) const
{
    // ex IntSimpleIndexableValue::toString
    return "#<array>";
}

void
game::interface::SimpleFunction::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntSimpleIndexableValue::store
    throw interpreter::Error::notSerializable();
}

