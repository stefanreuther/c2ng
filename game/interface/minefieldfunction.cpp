/**
  *  \file game/interface/minefieldfunction.cpp
  */

#include "game/interface/minefieldfunction.hpp"
#include "game/turn.hpp"
#include "game/game.hpp"
#include "interpreter/arguments.hpp"
#include "game/interface/minefieldcontext.hpp"

game::interface::MinefieldFunction::MinefieldFunction(Session& session)
    : m_session(session)
{ }

// IndexableValue:
afl::data::Value*
game::interface::MinefieldFunction::get(interpreter::Arguments& args)
{
    // ex int/if/mineif.h:IFMinefieldGet
    /* @q Minefield(id:Int):Obj (Function, Context)
       Access minefield properties.
       Use as
       | ForEach Minefield Do ...
       or
       | With Minefield(n) Do ...

       @diff This function was available for use in %With under the name %Minefields() since PCC 1.0.11.
       Do not use the name %Minefields in new code, it is not supported by PCC2; use %Minefield instead.

       @see int:index:group:minefieldproperty|Minefield Properties, int:index:group:minefieldcommand|Minefield Commands
       @since PCC 1.0.18, PCC2 1.99.8 */
    args.checkArgumentCount(1);

    int32_t mid;
    if (!interpreter::checkIntegerArg(mid, args.getNext())) {
        return 0;
    }

    return MinefieldContext::create(mid, m_session, false);
}

void
game::interface::MinefieldFunction::set(interpreter::Arguments& args, afl::data::Value* value)
{
    rejectSet(args, value);
}

// CallableValue:
int32_t
game::interface::MinefieldFunction::getDimension(int32_t which) const
{
    if (which == 0) {
        return 1;
    } else {
        if (Game* game = m_session.getGame().get()) {
            return game->currentTurn().universe().minefields().size();
        } else {
            return 0;
        }
    }
}

interpreter::Context*
game::interface::MinefieldFunction::makeFirstContext()
{
    Game* g = m_session.getGame().get();
    Root* r = m_session.getRoot().get();
    if (g != 0 || r != 0) {
        if (int mid = g->currentTurn().universe().minefields().findNextIndex(0)) {
            return new MinefieldContext(mid, *r, *g);
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

game::interface::MinefieldFunction*
game::interface::MinefieldFunction::clone() const
{
    return new MinefieldFunction(m_session);
}

// BaseValue:
String_t
game::interface::MinefieldFunction::toString(bool /*readable*/) const
{
    return "#<array:Minefield>";
}

void
game::interface::MinefieldFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
