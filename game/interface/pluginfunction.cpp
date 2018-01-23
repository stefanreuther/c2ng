/**
  *  \file game/interface/pluginfunction.cpp
  */

#include "game/interface/pluginfunction.hpp"
#include "interpreter/error.hpp"
#include "interpreter/arguments.hpp"

game::interface::PluginFunction::PluginFunction(Session& session)
    : m_session(session)
{ }

// IndexableValue:
game::interface::PluginContext*
game::interface::PluginFunction::get(interpreter::Arguments& args)
{
    /* @q System.Plugin(id:Str):Obj (Function)
       Accesses the properties of the plugin given by the name <tt>Id</tt>.
       If no such plugin was loaded, returns EMPTY.
       @see int:index:group:pluginproperty|Plugin Properties
       @since PCC2 1.99.25, PCC2 2.40.1 */
    args.checkArgumentCount(1);

    String_t arg;
    if (!interpreter::checkStringArg(arg, args.getNext())) {
        return 0;
    }

    return PluginContext::create(arg, m_session);
}

void
game::interface::PluginFunction::set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
{
    throw interpreter::Error::notAssignable();
}

// CallableValue:
int32_t
game::interface::PluginFunction::getDimension(int32_t /*which*/) const
{
    return 0;
}

interpreter::Context*
game::interface::PluginFunction::makeFirstContext()
{
    throw interpreter::Error::typeError(interpreter::Error::ExpectIterable);
}

game::interface::PluginFunction*
game::interface::PluginFunction::clone() const
{
    return new PluginFunction(m_session);
}

// BaseValue:
String_t
game::interface::PluginFunction::toString(bool /*readable*/) const
{
    return "#<plugin>";
}

void
game::interface::PluginFunction::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}
