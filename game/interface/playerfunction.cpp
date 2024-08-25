/**
  *  \file game/interface/playerfunction.cpp
  *  \brief Class game::interface::PlayerFunction
  */

#include "game/interface/playerfunction.hpp"
#include "game/game.hpp"
#include "game/interface/playercontext.hpp"
#include "game/root.hpp"
#include "interpreter/arguments.hpp"

/* @q Player(uid:Int):Obj (Function, Context)
   Access player properties such as other players' race names and scores.
   Use as
   | ForEach Player Do ...
   or
   | With Player(n) Do ...

   @diff This function was available for use in %With under the name %Players() since PCC 1.0.8.
   Do not use the name %Players in new code, it is not supported by PCC2; use %Player instead.

   @see int:index:group:playerproperty|Player Properties
   @since PCC 1.0.18, PCC2 1.99.8, PCC2 2.40 */

game::interface::PlayerFunction::PlayerFunction(Session& session)
    : m_session(session)
{ }

// IndexableValue:
interpreter::Context*
game::interface::PlayerFunction::get(interpreter::Arguments& args)
{
    // Check player number
    int32_t pid;
    args.checkArgumentCount(1);
    if (!interpreter::checkIntegerArg(pid, args.getNext())) {
        return 0;
    }

    return PlayerContext::create(pid, m_session);
}

void
game::interface::PlayerFunction::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    rejectSet(args, value);
}

// CallableValue:
size_t
game::interface::PlayerFunction::getDimension(size_t which) const
{
    // \change: This reports DIM(PLAYER)=13 in a v3 game, not 12 as PCC2.
    return which == 0
        ? 1
        : m_session.getRoot().get() != 0
        ? size_t(m_session.getRoot()->playerList().size())
        : 0;
}

interpreter::Context*
game::interface::PlayerFunction::makeFirstContext()
{
    // Valid state?
    Game* g = m_session.getGame().get();
    Root* r = m_session.getRoot().get();
    if (g == 0 || r == 0) {
        return 0;
    }

    // Find a player
    Player* pl = r->playerList().getFirstPlayer();
    while (pl != 0 && !pl->isReal()) {
        pl = r->playerList().getNextPlayer(pl);
    }

    if (pl != 0) {
        return new PlayerContext(pl->getId(), *g, *r, m_session.translator());
    } else {
        return 0;
    }
}

game::interface::PlayerFunction*
game::interface::PlayerFunction::clone() const
{
    return new PlayerFunction(m_session);
}

// BaseValue:
String_t
game::interface::PlayerFunction::toString(bool /*readable*/) const
{
    return "#<array>";
}

void
game::interface::PlayerFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
