/**
  *  \file client/si/widgetfunctionvalue.cpp
  */

#include "client/si/widgetfunctionvalue.hpp"
#include "client/si/scriptside.hpp"
#include "client/si/widgetreference.hpp"
#include "interpreter/error.hpp"

client::si::WidgetFunctionValue::WidgetFunctionValue(WidgetFunction func, game::Session& session, ScriptSide* ss, const WidgetReference& ref)
    : m_function(func),
      m_session(session),
      m_pScriptSide(ss),
      m_ref(ref)
{ }

// CallableValue:
afl::data::Value*
client::si::WidgetFunctionValue::get(interpreter::Arguments& args)
{
    if (ScriptSide* ss = m_pScriptSide.get()) {
        return callWidgetFunction(m_function, m_session, *ss, m_ref, args);
    } else {
        return 0;
    }
}

client::si::WidgetFunctionValue*
client::si::WidgetFunctionValue::clone() const
{
    return new WidgetFunctionValue(m_function, m_session, m_pScriptSide.get(), m_ref);
}
