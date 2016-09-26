/**
  *  \file game/interface/simpleprocedure.cpp
  */

#include "game/interface/simpleprocedure.hpp"
#include "interpreter/error.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"

game::interface::SimpleProcedure::SimpleProcedure(Session& session, Function_t func)
    : m_session(session),
      m_function(func)
{
    // ex IntSimpleProcedureValue::IntSimpleProcedureValue
}

game::interface::SimpleProcedure::~SimpleProcedure()
{ }

// CallableValue:
void
game::interface::SimpleProcedure::call(interpreter::Process& proc, afl::data::Segment& args, bool want_result)
{
    // ex IntSimpleProcedureValue::call
    interpreter::Arguments a(args, 0, args.size());
    m_function(proc, m_session, a);
    if (want_result) {
        proc.pushNewValue(0);
    }
}

bool
game::interface::SimpleProcedure::isProcedureCall()
{
    // ex IntSimpleProcedureValue::isProcedureCall
    return true;
}

int32_t
game::interface::SimpleProcedure::getDimension(int32_t /*which*/)
{
    // ex IntSimpleProcedureValue::getDimension
    return 0;
}

interpreter::Context*
game::interface::SimpleProcedure::makeFirstContext()
{
    // ex IntSimpleProcedureValue::makeFirstContext
    return 0;
}


// BaseValue:
String_t
game::interface::SimpleProcedure::toString(bool /*readable*/) const
{
    // ex IntSimpleProcedureValue::toString
    return "#<procedure>";
}

void
game::interface::SimpleProcedure::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext* /*ctx*/) const
{
    // ex IntSimpleProcedureValue::store
    throw interpreter::Error::notSerializable();
}

// Value:
game::interface::SimpleProcedure*
game::interface::SimpleProcedure::clone() const
{
    // ex IntSimpleProcedureValue::clone
    return new SimpleProcedure(m_session, m_function);
}
