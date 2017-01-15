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

// ProcedureValue:
void
game::interface::SimpleProcedure::call(interpreter::Process& proc, interpreter::Arguments& args)
{
    // ex IntSimpleProcedureValue::call
    m_function(proc, m_session, args);
}

// Value:
game::interface::SimpleProcedure*
game::interface::SimpleProcedure::clone() const
{
    // ex IntSimpleProcedureValue::clone
    return new SimpleProcedure(m_session, m_function);
}
