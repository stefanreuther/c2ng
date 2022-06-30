/**
  *  \file interpreter/simpleprocedure.cpp
  *  \brief Template class interpreter::SimpleProcedure
  */

#include "interpreter/simpleprocedure.hpp"

void
interpreter::SimpleProcedure<void>::call(Process& proc, Arguments& args)
{
    if (m_call) {
        m_call(proc, args);
    }
}

interpreter::ProcedureValue*
interpreter::SimpleProcedure<void>::clone() const
{
    return new SimpleProcedure<void>(m_call);
}
