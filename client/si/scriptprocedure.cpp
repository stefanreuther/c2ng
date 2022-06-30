/**
  *  \file client/si/scriptprocedure.cpp
  *  \brief Class client::si::ScriptProcedure
  */

#include "client/si/scriptprocedure.hpp"
#include "interpreter/error.hpp"
#include "client/si/scriptside.hpp"

// Constructor.
client::si::ScriptProcedure::ScriptProcedure(game::Session& session, ScriptSide* pScriptSide, void (*pFunction)(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args))
    : m_session(session),
      m_pScriptSide(pScriptSide),
      m_pFunction(pFunction)
{ }

// Destructor.
client::si::ScriptProcedure::~ScriptProcedure()
{ }


void
client::si::ScriptProcedure::call(interpreter::Process& proc, interpreter::Arguments& args)
{
    if (ScriptSide* ss = m_pScriptSide.get()) {
        m_pFunction(m_session, *ss, RequestLink1(proc, false), args);
    } else {
        throw interpreter::Error::contextError();
    }
}

client::si::ScriptProcedure*
client::si::ScriptProcedure::clone() const
{
    return new ScriptProcedure(m_session, m_pScriptSide.get(), m_pFunction);
}
