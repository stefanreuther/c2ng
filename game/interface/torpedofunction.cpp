/**
  *  \file game/interface/torpedofunction.cpp
  *  \brief Class game::interface::TorpedoFunction
  */

#include "game/interface/torpedofunction.hpp"
#include "interpreter/arguments.hpp"
#include "game/interface/torpedocontext.hpp"

/* @q Torpedo(id:Int):Obj (Function, Context)
   Access torpedo properties.
   Use as
   | ForEach Torpedo Do ...
   or
   | With Torpedo(n) Do ...

   @diff This function was available for use in %With under the name %Torpedoes() since PCC 1.0.6.
   Do not use the name %Torpedoes in new code, it is not supported by PCC2; use %Torpedo instead.

   @see int:index:group:torpedoproperty|Torpedo Properties, Launcher()
   @since PCC 1.0.18, PCC2 1.99.8 */

/* @q Launcher(id:Int):Obj (Function, Context)
   Access torpedo launcher properties.
   Use as
   | ForEach Launcher Do ...
   or
   | With Launcher(n) Do ...

   @diff This function was available for use in %With under the name %Launchers() since PCC 1.0.6.
   Do not use the name %Launchers in new code, it is not supported by PCC2; use %Launcher instead.

   @see int:index:group:torpedoproperty|Torpedo Properties, Torpedo()
   @since PCC 1.0.18, PCC2 1.99.8 */

game::interface::TorpedoFunction::TorpedoFunction(bool useLauncher, Session& session)
    : m_useLauncher(useLauncher),
      m_session(session)
{ }

// IndexableValue:
interpreter::Context*
game::interface::TorpedoFunction::get(interpreter::Arguments& args)
{
    // ex int/if/specif.h:IFTorpedoGet, IFLauncherGet
    int32_t id;
    args.checkArgumentCount(1);

    if (!interpreter::checkIntegerArg(id, args.getNext(), 1, int32_t(getDimension(1))-1)) {
        return 0;
    }
    return TorpedoContext::create(m_useLauncher, id, m_session);
}

void
game::interface::TorpedoFunction::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    rejectSet(args, value);
}

// CallableValue:
size_t
game::interface::TorpedoFunction::getDimension(size_t which) const
{
    // ex int/if/specif.h:IFTorpedoDim
    return (which == 0
            ? 1
            : (m_session.getShipList().get() != 0
               ? size_t(m_session.getShipList()->launchers().size()+1)
               : 0));
}

interpreter::Context*
game::interface::TorpedoFunction::makeFirstContext()
{
    // ex int/if/specif.h:IFTorpedoMake,IFLauncherMake
    if (game::spec::ShipList* list = m_session.getShipList().get()) {
        if (const game::spec::TorpedoLauncher* tl = list->launchers().findNext(0)) {
            return TorpedoContext::create(m_useLauncher, tl->getId(), m_session);
        }
    }
    return 0;
}

game::interface::TorpedoFunction*
game::interface::TorpedoFunction::clone() const
{
    return new TorpedoFunction(m_useLauncher, m_session);
}

// BaseValue:
String_t
game::interface::TorpedoFunction::toString(bool /*readable*/) const
{
    return "#<array>";
}

void
game::interface::TorpedoFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
