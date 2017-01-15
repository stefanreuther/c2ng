/**
  *  \file client/si/widgetfunctionvalue.cpp
  */

#include "client/si/widgetfunctionvalue.hpp"
#include "client/si/scriptside.hpp"
#include "interpreter/error.hpp"
#include "client/si/widgetreference.hpp"

client::si::WidgetFunctionValue::WidgetFunctionValue(WidgetFunction func, game::Session& session, ScriptSide* ss, const WidgetReference& ref)
    : m_function(func),
      m_session(session),
      m_pScriptSide(ss),
      m_ref(ref)
{ }

// BaseValue:
String_t
client::si::WidgetFunctionValue::toString(bool /*readable*/) const
{
    return "#<function>";
}

void
client::si::WidgetFunctionValue::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}

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

void
client::si::WidgetFunctionValue::set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
{
    throw interpreter::Error::notAssignable();
}

int32_t
client::si::WidgetFunctionValue::getDimension(int32_t /*which*/) const
{
    return 0;
}

interpreter::Context*
client::si::WidgetFunctionValue::makeFirstContext()
{
    throw interpreter::Error::typeError(interpreter::Error::ExpectIterable);
}

client::si::WidgetFunctionValue*
client::si::WidgetFunctionValue::clone() const
{
    return new WidgetFunctionValue(m_function, m_session, m_pScriptSide.get(), m_ref);
}
