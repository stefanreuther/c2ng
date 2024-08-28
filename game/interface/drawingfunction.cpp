/**
  *  \file game/interface/drawingfunction.cpp
  *  \brief Class game::interface::DrawingFunction
  */

#include "game/interface/drawingfunction.hpp"
#include "game/game.hpp"
#include "game/interface/drawingcontext.hpp"
#include "interpreter/arguments.hpp"

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
game::interface::DrawingFunction::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    rejectSet(args, value);
}

// CallableValue:
size_t
game::interface::DrawingFunction::getDimension(size_t /*which*/) const
{
    return 0;
}

game::interface::DrawingContext*
game::interface::DrawingFunction::makeFirstContext()
{
    if (Game* g = m_session.getGame().get()) {
        return DrawingContext::create(m_session, g->viewpointTurn());
    } else {
        return 0;
    }
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
game::interface::DrawingFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
