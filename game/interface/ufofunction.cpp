/**
  *  \file game/interface/ufofunction.cpp
  */

#include "game/interface/ufofunction.hpp"
#include "interpreter/error.hpp"
#include "interpreter/arguments.hpp"

/* @q Ufo(uid:Int):Obj (Function, Context)
   Access Ufo properties.
   Use as
   | ForEach Ufo Do ...
   or
   | With Ufo(n) Do ...
   @see int:index:group:ufoproperty|Ufo Properties, int:index:group:ufocommand|Ufo Commands */

game::interface::UfoFunction::UfoFunction(Session& session)
    : m_session(session)
{ }

// IndexableValue:
game::interface::UfoContext*
game::interface::UfoFunction::get(interpreter::Arguments& args)
{
    // ex int/if/ufoif.h:IFUfoGet
    // ex values.pas:CreateUfoContext
    int32_t id;
    args.checkArgumentCount(1, 1);
    if (!interpreter::checkIntegerArg(id, args.getNext(), 0, 32767)) {
        return 0;
    }

    Game* g = m_session.getGame().get();
    if (g == 0) {
        return 0;
    }
    Turn& t = g->currentTurn();

    Id_t index = t.universe().ufos().findIndexForId(id);
    if (index == 0) {
        return 0;
    }

    return new UfoContext(index, t, m_session);
}

void
game::interface::UfoFunction::set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
{
    throw interpreter::Error::notAssignable();
}

// CallableValue:
int32_t
game::interface::UfoFunction::getDimension(int32_t which) const
{
    // ex int/if/ufoif.h:IFUfoDim
    return which == 0 ? 1 : 32768;
}

game::interface::UfoContext*
game::interface::UfoFunction::makeFirstContext()
{
    // ex int/if/ufoif.h:IFUfoMake
    Game* g = m_session.getGame().get();
    if (g == 0) {
        return 0;
    }
    Turn& t = g->currentTurn();

    Id_t index = t.universe().ufos().findNextIndexNoWrap(0, false);
    if (index == 0) {
        return 0;
    }

    return new UfoContext(index, t, m_session);
}

game::interface::UfoFunction*
game::interface::UfoFunction::clone() const
{
    return new UfoFunction(m_session);
}

// BaseValue:
String_t
game::interface::UfoFunction::toString(bool /*readable*/) const
{
    // ex IntSimpleIndexableValue::toString
    return "#<array:Ufo>";
}

void
game::interface::UfoFunction::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}
