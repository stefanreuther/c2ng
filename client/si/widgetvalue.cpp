/**
  *  \file client/si/widgetvalue.cpp
  */

#include "client/si/widgetvalue.hpp"
#include "interpreter/error.hpp"

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
client::si::WidgetValue::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}

const client::si::WidgetReference&
client::si::WidgetValue::getValue() const
{
    return m_ref;
}
