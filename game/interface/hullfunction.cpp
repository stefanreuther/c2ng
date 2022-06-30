/**
  *  \file game/interface/hullfunction.cpp
  */

#include "game/interface/hullfunction.hpp"
#include "interpreter/arguments.hpp"
#include "game/interface/hullcontext.hpp"

/* @q Hull(id:Int):Obj (Function, Context)
   Access hull properties.
   Use as
   | ForEach Hull Do ...
   or
   | With Hull(n) Do ...

   @diff This function was available for use in %With under the name %Hulls() since PCC 1.0.6.
   Do not use the name %Hulls in new code, it is not supported by PCC2; use %Hull instead.

   @see int:index:group:hullproperty|Hull Properties
   @since PCC 1.0.18, PCC2 1.99.8, PCC2 2.40 */

game::interface::HullFunction::HullFunction(Session& session)
    : m_session(session)
{ }

// IndexableValue:
afl::data::Value*
game::interface::HullFunction::get(interpreter::Arguments& args)
{
    // ex int/if/hullif.h:IFHullGet
    int32_t id;
    args.checkArgumentCount(1);
    if (!interpreter::checkIntegerArg(id, args.getNext(), 1, getDimension(1)-1)) {
        return 0;
    }
    return HullContext::create(id, m_session);
}

void
game::interface::HullFunction::set(interpreter::Arguments& args, afl::data::Value* value)
{
    rejectSet(args, value);
}

// CallableValue:
int32_t
game::interface::HullFunction::getDimension(int32_t which) const
{
    // ex int/if/hullif.h:IFHullDim
    return (which == 0
            ? 1
            : (m_session.getShipList().get() != 0
               ? m_session.getShipList()->hulls().size()+1
               : 0));
}

interpreter::Context*
game::interface::HullFunction::makeFirstContext()
{
    // ex int/if/hullif.h:IFHullMake
    game::spec::ShipList* list = m_session.getShipList().get();
    Root* root = m_session.getRoot().get();
    if (list != 0 && root != 0 && list->hulls().size() > 0) {
        return new HullContext(1, *list, *m_session.getRoot());
    }
    return 0;
}

game::interface::HullFunction*
game::interface::HullFunction::clone() const
{
    return new HullFunction(m_session);
}

// BaseValue:
String_t
game::interface::HullFunction::toString(bool /*readable*/) const
{
    return "#<array>";
}

void
game::interface::HullFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
