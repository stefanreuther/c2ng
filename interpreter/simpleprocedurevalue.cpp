/**
  *  \file interpreter/simpleprocedurevalue.cpp
  */

#include "interpreter/simpleprocedurevalue.hpp"

interpreter::SimpleProcedureValue::SimpleProcedureValue(World& world, Call_t* call)
    : ProcedureValue(),
      m_world(world),
      m_call(call)
{ }

interpreter::SimpleProcedureValue::~SimpleProcedureValue()
{ }

void
interpreter::SimpleProcedureValue::call(Process& proc, Arguments& args)
{
    m_call(m_world, proc, args);
}

interpreter::SimpleProcedureValue*
interpreter::SimpleProcedureValue::clone() const
{
    return new SimpleProcedureValue(m_world, m_call);
}
