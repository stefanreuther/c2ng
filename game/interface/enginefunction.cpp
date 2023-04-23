/**
  *  \file game/interface/enginefunction.cpp
  *  \brief Class game::interface::EngineFunction
  */

#include "game/interface/enginefunction.hpp"
#include "game/interface/enginecontext.hpp"
#include "interpreter/arguments.hpp"

/* @q Engine(id:Int):Obj (Function, Context)
   Access engine weapon properties.
   Use as
   | ForEach Engine Do ...
   or
   | With Engine(n) Do ...

   @diff This function was available for use in %With under the name %Engines() since PCC 1.0.6.
   Do not use the name %Engines in new code, it is not supported by PCC2; use %Engine instead.

   @see int:index:group:engineproperty|Engine Properties
   @since PCC 1.0.18, PCC2 1.99.8, PCC2 2.40 */

game::interface::EngineFunction::EngineFunction(Session& session)
    : m_session(session)
{ }

// IndexableValue:
interpreter::Context*
game::interface::EngineFunction::get(interpreter::Arguments& args)
{
    // ex int/if/specif.h:IFEngineGet
    int32_t id;
    args.checkArgumentCount(1);
    if (!interpreter::checkIntegerArg(id, args.getNext(), 1, getDimension(1)-1)) {
        return 0;
    }

    return EngineContext::create(id, m_session);
}

void
game::interface::EngineFunction::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    rejectSet(args, value);
}

// CallableValue:
int32_t
game::interface::EngineFunction::getDimension(int32_t which) const
{
    // ex int/if/specif.h:IFEngineDim
    return (which == 0
            ? 1
            : (m_session.getShipList().get() != 0
               ? m_session.getShipList()->engines().size()+1
               : 0));
}

interpreter::Context*
game::interface::EngineFunction::makeFirstContext()
{
    // ex int/if/specif.h:IFEngineMake
    if (game::spec::ShipList* list = m_session.getShipList().get()) {
        if (const game::spec::Engine* e = list->engines().findNext(0)) {
            return new EngineContext(e->getId(), *list);
        }
    }
    return 0;
}

game::interface::EngineFunction*
game::interface::EngineFunction::clone() const
{
    return new EngineFunction(m_session);
}

// BaseValue:
String_t
game::interface::EngineFunction::toString(bool /*readable*/) const
{
    return "#<array>";
}

void
game::interface::EngineFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
