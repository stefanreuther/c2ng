/**
  *  \file game/interface/inboxfunction.cpp
  */

#include "game/interface/inboxfunction.hpp"
#include "interpreter/error.hpp"
#include "game/actions/preconditions.hpp"
#include "game/interface/inboxcontext.hpp"
#include "interpreter/arguments.hpp"
#include "game/turn.hpp"
#include "game/root.hpp"

using interpreter::Error;

/* @q InMsg(n:Int):Obj (Function, Context)
   Access incoming message.
   Use as
   | ForEach InMsg Do ...
   or
   | With InMsg(n) Do ...

   The parameter %n runs from 1 to {My.InMsgs}.
   @see int:index:group:incomingmessageproperty|Incoming Message Properties
   @since PCC2 1.99.13, PCC 1.1.13 */

game::interface::InboxFunction::InboxFunction(Session& session)
    : m_session(session)
{ }

game::interface::InboxFunction::~InboxFunction()
{ }

// IndexableValue:
afl::data::Value*
game::interface::InboxFunction::get(interpreter::Arguments& args)
{
    // ex IFInmsgGet
    // ex msgint.pas:CreateInMsgContext
    args.checkArgumentCount(1);

    afl::base::Ptr<Game> g = m_session.getGame();
    afl::base::Ptr<Root> r = m_session.getRoot();
    if (g.get() == 0 || r.get() == 0) {
        return 0;
    }

    // FIXME: use checkIndexArg
    int32_t index;
    if (!interpreter::checkIntegerArg(index, args.getNext(), 1, int32_t(g->currentTurn().inbox().getNumMessages()))) {
        return 0;
    }

    return new InboxContext(size_t(index-1), m_session.translator(), *r, *g);
}

void
game::interface::InboxFunction::set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
{
    throw Error::notAssignable();
}

// CallableValue:
int32_t
game::interface::InboxFunction::getDimension(int32_t which) const
{
    // ex IFInmsgDim
    afl::base::Ptr<Game> g = m_session.getGame();
    afl::base::Ptr<Root> r = m_session.getRoot();
    if (g.get() == 0 || r.get() == 0) {
        return 0;
    } else if (which == 0) {
        return 1;
    } else {
        return int32_t(g->currentTurn().inbox().getNumMessages());
    }
}

interpreter::Context*
game::interface::InboxFunction::makeFirstContext()
{
    // ex IFInmsgMake
    afl::base::Ptr<Game> g = m_session.getGame();
    afl::base::Ptr<Root> r = m_session.getRoot();
    if (g.get() == 0 || r.get() == 0) {
        return 0;
    } else if (g->currentTurn().inbox().getNumMessages() == 0) {
        return 0;
    } else {
        return new InboxContext(0, m_session.translator(), *r, *g);
    }
}

// BaseValue:
String_t
game::interface::InboxFunction::toString(bool /*readable*/) const
{
    return "#<array>";
}

void
game::interface::InboxFunction::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
{
    throw Error::notSerializable();
}

game::interface::InboxFunction*
game::interface::InboxFunction::clone() const
{
    return new InboxFunction(m_session);
}
