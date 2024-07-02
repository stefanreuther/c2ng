/**
  *  \file game/interface/inboxfunction.cpp
  *  \brief Class game::interface::InboxFunction
  */

#include "game/interface/inboxfunction.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/inboxcontext.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"

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
interpreter::Context*
game::interface::InboxFunction::get(interpreter::Arguments& args)
{
    // ex IFInmsgGet
    // ex msgint.pas:CreateInMsgContext
    args.checkArgumentCount(1);

    afl::base::Ptr<Game> g = m_session.getGame();
    if (g.get() == 0) {
        return 0;
    }

    size_t index;
    if (!interpreter::checkIndexArg(index, args.getNext(), 1, g->viewpointTurn().inbox().getNumMessages())) {
        return 0;
    }

    return new InboxContext(index, m_session, g->viewpointTurn());
}

void
game::interface::InboxFunction::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    rejectSet(args, value);
}

// CallableValue:
int32_t
game::interface::InboxFunction::getDimension(int32_t which) const
{
    // ex IFInmsgDim
    afl::base::Ptr<Game> g = m_session.getGame();
    afl::base::Ptr<Root> r = m_session.getRoot();
    if (which == 0) {
        return 1;
    } else if (g.get() == 0 || r.get() == 0) {
        return 0;
    } else {
        return int32_t(g->viewpointTurn().inbox().getNumMessages()) + 1;
    }
}

interpreter::Context*
game::interface::InboxFunction::makeFirstContext()
{
    // ex IFInmsgMake
    afl::base::Ptr<Game> g = m_session.getGame();
    if (g.get() == 0) {
        return 0;
    } else if (g->viewpointTurn().inbox().getNumMessages() == 0) {
        return 0;
    } else {
        return new InboxContext(0, m_session, g->viewpointTurn());
    }
}

// BaseValue:
String_t
game::interface::InboxFunction::toString(bool /*readable*/) const
{
    return "#<array>";
}

void
game::interface::InboxFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

game::interface::InboxFunction*
game::interface::InboxFunction::clone() const
{
    return new InboxFunction(m_session);
}
