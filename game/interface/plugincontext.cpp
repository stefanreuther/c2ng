/**
  *  \file game/interface/plugincontext.cpp
  */

#include "game/interface/plugincontext.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "afl/string/format.hpp"
#include "game/interface/pluginproperty.hpp"

namespace {
    enum PluginDomain { PluginPropertyDomain };

    const interpreter::NameTable property_mapping[] = {
        { "DESCRIPTION",        game::interface::ipiDescription,   PluginPropertyDomain, interpreter::thString },
        { "DIRECTORY",          game::interface::ipiBaseDirectory, PluginPropertyDomain, interpreter::thString },
        { "ID",                 game::interface::ipiId,            PluginPropertyDomain, interpreter::thString },
        { "NAME",               game::interface::ipiName,          PluginPropertyDomain, interpreter::thString },
    };
}


game::interface::PluginContext::PluginContext(String_t name, Session& session)
    : m_name(name),
      m_session(session)
{
    // ex IntPluginContext::IntPluginContext
}

game::interface::PluginContext::~PluginContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::PluginContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntPluginContext::lookup
    return lookupName(name, property_mapping, result) ? this : 0;
}

afl::data::Value*
game::interface::PluginContext::get(PropertyIndex_t index)
{
    if (util::plugin::Plugin* plug = m_session.plugins().getPluginById(m_name)) {
        return getPluginProperty(*plug, PluginProperty(property_mapping[index].index));
    } else {
        return 0;
    }
}

bool
game::interface::PluginContext::next()
{
    // ex IntPluginContext::next
    return false;
}

game::interface::PluginContext*
game::interface::PluginContext::clone() const
{
    // ex IntPluginContext::clone
    return new PluginContext(m_name, m_session);
}

game::map::Object*
game::interface::PluginContext::getObject()
{
    // ex IntPluginContext::getObject
    return 0;
}

void
game::interface::PluginContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
{
    // ex IntPluginContext::enumProperties
    acceptor.enumTable(property_mapping);
}

// BaseValue:
String_t
game::interface::PluginContext::toString(bool /*readable*/) const
{
    // ex IntPluginContext::toString
    return afl::string::Format("#<plugin:%s>", m_name);
}

void
game::interface::PluginContext::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntPluginContext::store
    throw interpreter::Error::notSerializable();
}

game::interface::PluginContext*
game::interface::PluginContext::create(String_t name, Session& session)
{
    name = afl::string::strUCase(name);
    if (session.plugins().getPluginById(name) != 0) {
        return new PluginContext(name, session);
    } else {
        return 0;
    }
}
