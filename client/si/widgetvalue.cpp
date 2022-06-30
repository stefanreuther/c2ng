/**
  *  \file client/si/widgetvalue.cpp
  */

#include "client/si/widgetvalue.hpp"

client::si::WidgetValue::WidgetValue(const WidgetReference& ref)
    : m_ref(ref)
{ }

game::map::Object*
client::si::WidgetValue::getObject()
{
    return 0;
}

String_t
client::si::WidgetValue::toString(bool /*readable*/) const
{
    return "#<widget>";
}

void
client::si::WidgetValue::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

const client::si::WidgetReference&
client::si::WidgetValue::getValue() const
{
    return m_ref;
}
