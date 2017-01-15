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

// BaseValue:
String_t
client::si::ScriptProcedure::toString(bool /*readable*/) const
{
    return "#<procedure>";
}

void
client::si::ScriptProcedure::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}

// CallableValue:
void
client::si::ScriptProcedure::call(interpreter::Process& proc, afl::data::Segment& args, bool want_result)
{
    if (ScriptSide* ss = m_pScriptSide.get()) {
        // FIXME: do we need the want_result? If not, we can make ScriptProcedure a simpler ProcedureValue instead.
        interpreter::Arguments a(args, 0, args.size());
        m_pFunction(m_session, *ss, RequestLink1(proc, want_result), a);
    } else {
        throw interpreter::Error::contextError();
    }
}

bool
client::si::ScriptProcedure::isProcedureCall() const
{
    return true;
}

int32_t
client::si::ScriptProcedure::getDimension(int32_t /*which*/) const
{
    return 0;
}

interpreter::Context*
client::si::ScriptProcedure::makeFirstContext()
{
    throw interpreter::Error::typeError(interpreter::Error::ExpectIterable);
}

client::si::ScriptProcedure*
client::si::ScriptProcedure::clone() const
{
    return new ScriptProcedure(m_session, m_pScriptSide.get(), m_pFunction);
}
