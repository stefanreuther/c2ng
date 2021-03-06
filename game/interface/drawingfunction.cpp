/**
  *  \file game/interface/drawingfunction.cpp
  */

#include "game/interface/drawingfunction.hpp"
#include "interpreter/arguments.hpp"
#include "game/interface/drawingcontext.hpp"

game::interface::DrawingFunction::DrawingFunction(Session& session)
    : m_session(session)
{ }

// IndexableValue:
afl::data::Value*
game::interface::DrawingFunction::get(interpreter::Arguments& /*args*/)
{
    throw interpreter::Error::typeError(interpreter::Error::ExpectIndexable);
}

void
game::interface::DrawingFunction::set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
{
    // ex IntSimpleIndexableValue::set
    throw interpreter::Error::notAssignable();
}

// CallableValue:
int32_t
game::interface::DrawingFunction::getDimension(int32_t /*which*/) const
{
    return 0;
}

game::interface::DrawingContext*
game::interface::DrawingFunction::makeFirstContext()
{
    return DrawingContext::create(m_session);
}

game::interface::DrawingFunction*
game::interface::DrawingFunction::clone() const
{
    return new DrawingFunction(m_session);
}

// BaseValue:
String_t
game::interface::DrawingFunction::toString(bool /*readable*/) const
{
    return "#<array:Marker>";
}

void
game::interface::DrawingFunction::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}
