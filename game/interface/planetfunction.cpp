/**
  *  \file game/interface/planetfunction.cpp
  */

#include "game/interface/planetfunction.hpp"
#include "interpreter/arguments.hpp"
#include "game/turn.hpp"
#include "game/game.hpp"
#include "game/interface/planetcontext.hpp"
#include "interpreter/error.hpp"
#include "game/map/anyplanettype.hpp"

game::interface::PlanetFunction::PlanetFunction(Session& session)
    : m_session(session)
{ }


// IndexableValue:
game::interface::PlanetContext*
game::interface::PlanetFunction::get(interpreter::Arguments& args)
{
    /* @q Planet(sid:Int):Obj (Function, Context)
       Access planet (and starbase) properties.
       Use as
       | ForEach Planet Do ...
       or
       | With Planet(n) Do ...

       @diff This function was available for use in %With under the name %Planets() since PCC 1.0.6.
       Do not use the name %Planets in new code, it is not supported by PCC2; use %Planet instead.

       @see int:index:group:planetproperty|Planet Properties, int:index:group:planetcommand|Planet Commands
       @since PCC 1.0.18, PCC2 1.99.8 */
    // ex IntSimpleIndexableValue::get, int/if/planetif.h:IFPlanetGet
    args.checkArgumentCount(1);

    int32_t id;
    if (!interpreter::checkIntegerArg(id, args.getNext())) {
        return 0;
    }

    return PlanetContext::create(id, m_session);
}

void
game::interface::PlanetFunction::set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
{
    // ex IntSimpleIndexableValue::set
    throw interpreter::Error::notAssignable();
}


// CallableValue:
int32_t
game::interface::PlanetFunction::getDimension(int32_t which) const
{
    // ex int/if/planetif.h:IFPlanetDim
    if (which == 0) {
        return 1;
    } else {
        if (Game* game = m_session.getGame().get()) {
            return game->currentTurn().universe().planets().size();
        } else {
            return 0;
        }
    }
}

interpreter::Context*
game::interface::PlanetFunction::makeFirstContext()
{
    Game* game = m_session.getGame().get();
    Root* root = m_session.getRoot().get();
    if (game != 0 && root != 0) {
        Id_t id = game::map::AnyPlanetType(game->currentTurn().universe().planets()).findNextIndex(0);
        if (id != 0) {
            return new PlanetContext(id, m_session, *root, *game);
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

game::interface::PlanetFunction*
game::interface::PlanetFunction::clone() const
{
    return new PlanetFunction(m_session);
}

// BaseValue:
String_t
game::interface::PlanetFunction::toString(bool /*readable*/) const
{
    // ex IntSimpleIndexableValue::toString
    return "#<array:Planet>";
}

void
game::interface::PlanetFunction::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}
