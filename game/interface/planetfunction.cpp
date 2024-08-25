/**
  *  \file game/interface/planetfunction.cpp
  *  \brief Class game::interface::PlanetFunction
  */

#include "game/interface/planetfunction.hpp"
#include "game/game.hpp"
#include "game/interface/planetcontext.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"

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

    if (Game* g = m_session.getGame().get()) {
        return PlanetContext::create(id, m_session, *g, g->viewpointTurn());
    } else {
        return 0;
    }
}

void
game::interface::PlanetFunction::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    // ex IntSimpleIndexableValue::set
    rejectSet(args, value);
}


// CallableValue:
size_t
game::interface::PlanetFunction::getDimension(size_t which) const
{
    // ex int/if/planetif.h:IFPlanetDim
    if (which == 0) {
        return 1;
    } else {
        if (Game* game = m_session.getGame().get()) {
            return size_t(game->viewpointTurn().universe().planets().size() + 1);
        } else {
            return 0;
        }
    }
}

interpreter::Context*
game::interface::PlanetFunction::makeFirstContext()
{
    if (Game* g = m_session.getGame().get()) {
        return PlanetContext::create(g->viewpointTurn().universe().allPlanets().findNextIndex(0), m_session, *g, g->viewpointTurn());
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
game::interface::PlanetFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
