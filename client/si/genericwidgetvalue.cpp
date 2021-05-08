/**
  *  \file client/si/genericwidgetvalue.cpp
  */

#include "client/si/genericwidgetvalue.hpp"
#include "interpreter/error.hpp"
#include "client/si/widgetcommandvalue.hpp"
#include "client/si/widgetfunctionvalue.hpp"
#include "client/si/scriptside.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "client/si/widgetproperty.hpp"
#include "client/si/widgetreference.hpp"

client::si::GenericWidgetValue::GenericWidgetValue(afl::base::Memory<const interpreter::NameTable> names, game::Session& session,
                                                   ScriptSide* ss, const WidgetReference& ref)
    : WidgetValue(ref),
      m_names(names),
      m_session(session),
      m_pScriptSide(ss)
{ }

interpreter::Context*
client::si::GenericWidgetValue::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    return lookupName(name, m_names, result) ? this : 0;
}

void
client::si::GenericWidgetValue::set(PropertyIndex_t index, const afl::data::Value* value)
{
    if (const interpreter::NameTable* pe = m_names.at(index)) {
        switch (GenericWidgetDomain(pe->domain)) {
         case WidgetCommandDomain:
         case WidgetFunctionDomain:
            throw interpreter::Error::notAssignable();
         case WidgetPropertyDomain:
            if (ScriptSide* ss = m_pScriptSide.get()) {
                setWidgetProperty(WidgetProperty(pe->index), value, *ss, getValue());
            } else {
                throw interpreter::Error::notAssignable();
            }
            break;
        }
    } else {
        // Cannot happen
        throw interpreter::Error::notAssignable();
    }
}

afl::data::Value*
client::si::GenericWidgetValue::get(PropertyIndex_t index)
{
    if (const interpreter::NameTable* pe = m_names.at(index)) {
        switch (GenericWidgetDomain(pe->domain)) {
         case WidgetCommandDomain:
            return new WidgetCommandValue(WidgetCommand(pe->index), m_session, m_pScriptSide.get(), getValue());
         case WidgetFunctionDomain:
            return new WidgetFunctionValue(WidgetFunction(pe->index), m_session, m_pScriptSide.get(), getValue());
         case WidgetPropertyDomain:
            if (ScriptSide* ss = m_pScriptSide.get()) {
                return getWidgetProperty(WidgetProperty(pe->index), *ss, getValue());
            } else {
                return 0;
            }
        }
    }
    return 0;
}

client::si::GenericWidgetValue*
client::si::GenericWidgetValue::clone() const
{
    return new GenericWidgetValue(*this);
}

void
client::si::GenericWidgetValue::enumProperties(interpreter::PropertyAcceptor& acceptor)
{
    acceptor.enumTable(m_names);
}
