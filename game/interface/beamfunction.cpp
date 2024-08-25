/**
  *  \file game/interface/beamfunction.cpp
  *  \brief Class game::interface::BeamFunction
  */

#include "game/interface/beamfunction.hpp"
#include "game/interface/beamcontext.hpp"
#include "interpreter/arguments.hpp"

/* @q Beam(id:Int):Obj (Function, Context)
   Access beam weapon properties.
   Use as
   | ForEach Beam Do ...
   or
   | With Beam(n) Do ...

   @diff This function was available for use in %With under the name %Beams() since PCC 1.0.6.
   Do not use the name %Beams in new code, it is not supported by PCC2; use %Beam instead.

   @see int:index:group:beamproperty|Beam Properties
   @since PCC 1.0.18, PCC2 1.99.8, PCC2 2.40 */

game::interface::BeamFunction::BeamFunction(Session& session)
    : m_session(session)
{ }

// IndexableValue:
interpreter::Context*
game::interface::BeamFunction::get(interpreter::Arguments& args)
{
    // ex int/if/specif.h:IFBeamGet
    int32_t id;
    args.checkArgumentCount(1);
    if (!interpreter::checkIntegerArg(id, args.getNext(), 1, int32_t(getDimension(1)-1))) {
        return 0;
    }

    return BeamContext::create(id, m_session);
}

void
game::interface::BeamFunction::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    rejectSet(args, value);
}

// CallableValue:
size_t
game::interface::BeamFunction::getDimension(size_t which) const
{
    // ex int/if/specif.h:IFBeamDim
    return (which == 0
            ? 1
            : (m_session.getShipList().get() != 0
               ? size_t(m_session.getShipList()->beams().size()+1)
               : 0));
}

interpreter::Context*
game::interface::BeamFunction::makeFirstContext()
{
    // ex int/if/specif.h:IFBeamMake
    if (game::spec::ShipList* list = m_session.getShipList().get()) {
        if (const game::spec::Beam* firstBeam = list->beams().findNext(0)) {
            return BeamContext::create(firstBeam->getId(), m_session);
        }
    }
    return 0;
}

game::interface::BeamFunction*
game::interface::BeamFunction::clone() const
{
    return new BeamFunction(m_session);
}

// BaseValue:
String_t
game::interface::BeamFunction::toString(bool /*readable*/) const
{
    return "#<array>";
}

void
game::interface::BeamFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
