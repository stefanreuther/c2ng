/**
  *  \file game/interface/shipfunction.cpp
  */

#include "game/interface/shipfunction.hpp"
#include "interpreter/arguments.hpp"
#include "game/turn.hpp"
#include "game/game.hpp"
#include "game/interface/shipcontext.hpp"
#include "interpreter/error.hpp"
#include "game/map/anyshiptype.hpp"

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

    return ShipContext::create(id, m_session);
}

void
game::interface::ShipFunction::set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
{
    // ex IntSimpleIndexableValue::set
    throw interpreter::Error::notAssignable();
}


// CallableValue:
int32_t
game::interface::ShipFunction::getDimension(int32_t which) const
{
    // ex int/if/shipif.h:IFShipDim
    if (which == 0) {
        return 1;
    } else {
        if (Game* game = m_session.getGame().get()) {
            return game->currentTurn().universe().ships().size();
        } else {
            return 0;
        }
    }
}

game::interface::ShipContext*
game::interface::ShipFunction::makeFirstContext()
{
    Game* game = m_session.getGame().get();
    Root* root = m_session.getRoot().get();
    game::spec::ShipList* shipList = m_session.getShipList().get();
    if (game != 0 && root != 0 && shipList != 0) {
        Id_t id = game::map::AnyShipType(game->currentTurn().universe().ships()).findNextIndex(0);
        if (id != 0) {
            return new ShipContext(id, m_session, *root, *game, *shipList);
        } else {
            return 0;
        }
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
game::interface::ShipFunction::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}
