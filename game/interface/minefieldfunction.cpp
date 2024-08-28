/**
  *  \file game/interface/minefieldfunction.cpp
  *  \brief Class game::interface::MinefieldFunction
  */

#include "game/interface/minefieldfunction.hpp"
#include "game/game.hpp"
#include "game/interface/minefieldcontext.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"

game::interface::MinefieldFunction::MinefieldFunction(Session& session)
    : m_session(session)
{ }

// IndexableValue:
interpreter::Context*
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

    if (Game* g = m_session.getGame().get()) {
        return MinefieldContext::create(mid, m_session, *g, g->viewpointTurn(), false);
    } else {
        return 0;
    }
}

void
game::interface::MinefieldFunction::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    rejectSet(args, value);
}

// CallableValue:
size_t
game::interface::MinefieldFunction::getDimension(size_t which) const
{
    if (which == 0) {
        return 1;
    } else {
        if (Game* game = m_session.getGame().get()) {
            return size_t(game->viewpointTurn().universe().minefields().size()+1);
        } else {
            return 0;
        }
    }
}

interpreter::Context*
game::interface::MinefieldFunction::makeFirstContext()
{
    if (Game* g = m_session.getGame().get()) {
        return MinefieldContext::create(g->viewpointTurn().universe().minefields().findNextIndex(0), m_session, *g, g->viewpointTurn(), false);
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
