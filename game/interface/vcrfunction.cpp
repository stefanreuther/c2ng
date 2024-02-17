/**
  *  \file game/interface/vcrfunction.cpp
  *  \brief Class game::interface::VcrFunction
  */

#include <algorithm>
#include "game/interface/vcrfunction.hpp"
#include "interpreter/arguments.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"

/* @q Vcr(uid:Int):Obj (Function, Context)
   Access properties of combat recordings.
   Use as
   | ForEach Vcr Do ...
   or
   | With Vcr(n) Do ...
   @see int:index:group:combatproperty|Combat Properties */

game::interface::VcrFunction::VcrFunction(Session& session)
    : m_session(session)
{ }

// IndexableValue:
game::interface::VcrContext*
game::interface::VcrFunction::get(interpreter::Arguments& args)
{
    args.checkArgumentCount(1);
    int32_t i;
    if (!interpreter::checkIntegerArg(i, args.getNext(), 1, getNumBattles())) {
        return 0;
    }

    // OK, build result. Note that the user indexes are 1-based!
    if (Game* g = m_session.getGame().get()) {
        return VcrContext::create(i-1, m_session, g->viewpointTurn().getBattles());
    } else {
        return 0;
    }
}

void
game::interface::VcrFunction::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    rejectSet(args, value);
}

// CallableValue:
int32_t
game::interface::VcrFunction::getDimension(int32_t which) const
{
    if (which == 0) {
        return 1;
    } else {
        return getNumBattles() + 1;
    }
}

game::interface::VcrContext*
game::interface::VcrFunction::makeFirstContext()
{
    if (Game* g = m_session.getGame().get()) {
        return VcrContext::create(0, m_session, g->viewpointTurn().getBattles());
    } else {
        return 0;
    }
}

game::interface::VcrFunction*
game::interface::VcrFunction::clone() const
{
    return new VcrFunction(m_session);
}

// BaseValue:
String_t
game::interface::VcrFunction::toString(bool /*readable*/) const
{
    return "#<array:Vcr>";
}

void
game::interface::VcrFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

int32_t
game::interface::VcrFunction::getNumBattles() const
{
    if (Game* g = m_session.getGame().get()) {
        if (game::vcr::Database* db = g->viewpointTurn().getBattles().get()) {
            return int32_t(std::min(db->getNumBattles(), size_t(0x7FFFFFFE)));
        }
    }
    return 0;
}
