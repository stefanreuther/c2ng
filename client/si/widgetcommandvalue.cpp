/**
  *  \file client/si/widgetcommandvalue.cpp
  */

#include "client/si/widgetcommandvalue.hpp"
#include "client/si/scriptside.hpp"

client::si::WidgetCommandValue::WidgetCommandValue(WidgetCommand cmd, game::Session& session, ScriptSide* ss, const WidgetReference& ref)
    : m_command(cmd),
      m_session(session),
      m_pScriptSide(ss),
      m_ref(ref)
{ }

void
client::si::WidgetCommandValue::call(interpreter::Process& proc, interpreter::Arguments& args)
{
    if (ScriptSide* ss = m_pScriptSide.get()) {
        callWidgetCommand(m_command, m_session, *ss, m_ref, proc, args);
    } else {
        throw interpreter::Error::contextError();
    }
}

client::si::WidgetCommandValue*
client::si::WidgetCommandValue::clone() const
{
    return new WidgetCommandValue(*this);
}
