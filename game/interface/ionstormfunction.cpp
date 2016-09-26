/**
  *  \file game/interface/ionstormfunction.cpp
  */

#include "game/interface/ionstormfunction.hpp"
#include "game/game.hpp"
#include "game/interface/ionstormcontext.hpp"
#include "game/map/universe.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/values.hpp"

// /* @q Storm(id:Int):Obj (Function, Context)
//    Access ion storm properties.
//    Use as
//    | ForEach Storm Do ...
//    or
//    | With Storm(n) Do ...

//    @diff This function was available for use in %With under the name %Storms() since PCC 1.0.11.
//    Do not use the name %Storms in new code, it is not supported by PCC2; use %Storm instead.

//    @see int:index:group:stormproperty|Storm Properties, int:index:group:stormcommand|Storm Commands
//    @since PCC 1.0.18, PCC2 1.99.8 */

game::interface::IonStormFunction::IonStormFunction(Session& session)
    : m_session(session)
{ }

// IndexableValue:
afl::data::Value*
game::interface::IonStormFunction::get(interpreter::Arguments& args)
{
    // ex IFIonGet
    args.checkArgumentCount(1);

    int32_t id;
    if (!interpreter::checkIntegerArg(id, args.getNext())) {
        return 0;
    }

    Game* game = m_session.getGame().get();
    if (game != 0 && game->currentTurn().universe().ionStorms().get(id) != 0) {
        return new IonStormContext(id, m_session, game);
    } else {
        return 0;
    }
}

void
game::interface::IonStormFunction::set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
{
    // ex IntSimpleIndexableValue::set
    throw interpreter::Error::notAssignable();
}

// CallableValue:
int32_t
game::interface::IonStormFunction::getDimension(int32_t which)
{
    // ex int/if/ionif.h:IFIonDim
    if (which == 0) {
        return 0;
    } else if (Game* g = m_session.getGame().get()) {
        return g->currentTurn().universe().ionStorms().size();
    } else {
        return 0;
    }
}

interpreter::Context*
game::interface::IonStormFunction::makeFirstContext()
{
    Game* game = m_session.getGame().get();
    if (game != 0) {
        int id = game->currentTurn().universe().ionStormType().findNextIndex(0);
        if (id != 0) {
            return new IonStormContext(id, m_session, game);
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

game::interface::IonStormFunction*
game::interface::IonStormFunction::clone() const
{
    return new IonStormFunction(m_session);
}

// BaseValue:
String_t
game::interface::IonStormFunction::toString(bool /*readable*/) const
{
    // ex IntSimpleIndexableValue::toString
    return "#<array:Storm>";
}

void
game::interface::IonStormFunction::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext* /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}
