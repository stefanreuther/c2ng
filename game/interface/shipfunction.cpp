/**
  *  \file game/interface/shipfunction.cpp
  *  \brief Class game::interface::ShipFunction
  */

#include "game/interface/shipfunction.hpp"
#include "game/game.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"

game::interface::ShipFunction::ShipFunction(Session& session)
    : m_session(session)
{ }


// IndexableValue:
game::interface::ShipContext*
game::interface::ShipFunction::get(interpreter::Arguments& args)
{
    /* @q Ship(sid:Int):Obj (Function, Context)
       Access starship properties.
       Use as
       | ForEach Ship Do ...
       or
       | With Ship(n) Do ...

       @diff This function was available for use in %With under the name %Ships() since PCC 1.0.6.
       Do not use the name %Ships in new code, it is not supported by PCC2; use %Ship instead.

       @see int:index:group:shipproperty|Ship Properties, int:index:group:shipcommand|Ship Commands
       @since PCC 1.0.18, PCC2 1.99.8 */
    // ex IntSimpleIndexableValue::get, int/if/shipif.h:IFShipGet
    args.checkArgumentCount(1);

    int32_t id;
    if (!interpreter::checkIntegerArg(id, args.getNext())) {
        return 0;
    }

    if (Game* g = m_session.getGame().get()) {
        return ShipContext::create(id, m_session, *g, g->viewpointTurn());
    } else {
        return 0;
    }
}

void
game::interface::ShipFunction::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    rejectSet(args, value);
}


// CallableValue:
size_t
game::interface::ShipFunction::getDimension(size_t which) const
{
    // ex int/if/shipif.h:IFShipDim
    if (which == 0) {
        return 1;
    } else {
        if (Game* game = m_session.getGame().get()) {
            return size_t(game->viewpointTurn().universe().ships().size() + 1);
        } else {
            return 0;
        }
    }
}

game::interface::ShipContext*
game::interface::ShipFunction::makeFirstContext()
{
    if (Game* g = m_session.getGame().get()) {
        return ShipContext::create(g->viewpointTurn().universe().allShips().findNextIndex(0), m_session, *g, g->viewpointTurn());
    } else {
        return 0;
    }
}

game::interface::ShipFunction*
game::interface::ShipFunction::clone() const
{
    return new ShipFunction(m_session);
}

// BaseValue:
String_t
game::interface::ShipFunction::toString(bool /*readable*/) const
{
    // ex IntSimpleIndexableValue::toString
    return "#<array:Ship>";
}

void
game::interface::ShipFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
