/**
  *  \file game/interface/explosionfunction.cpp
  */

#include "game/interface/explosionfunction.hpp"
#include "interpreter/error.hpp"

/* @q Explosion:Any (Context)
   @noproto
   | ForEach Explosion Do ...
   | Find(Explosion, ...)

   The <tt>Explosion</tt> array contains all current explosion reports.

   Because explosions have no Id, therefore it is not possible to access a specific explosion.
   The <tt>Explosion</tt> array can only be used to iterate over,
   for example, using the {ForEach} command or the {Find} function.

   @since PCC2 2.40.1 */
game::interface::ExplosionFunction::ExplosionFunction(Session& session)
    : m_session(session)
{ }

// IndexableValue:
game::interface::ExplosionContext*
game::interface::ExplosionFunction::get(interpreter::Arguments& /*args*/)
{
    throw interpreter::Error::typeError(interpreter::Error::ExpectIndexable);
}

void
game::interface::ExplosionFunction::set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
{
    throw interpreter::Error::notAssignable();
}

// CallableValue:
int32_t
game::interface::ExplosionFunction::getDimension(int32_t /*which*/) const
{
    return 0;
}

game::interface::ExplosionContext*
game::interface::ExplosionFunction::makeFirstContext()
{
    // The first explosion always is 1. If there is no first explosion, this returns null.
    return ExplosionContext::create(1, m_session);
}

game::interface::ExplosionFunction*
game::interface::ExplosionFunction::clone() const
{
    return new ExplosionFunction(m_session);
}

// BaseValue:
String_t
game::interface::ExplosionFunction::toString(bool /*readable*/) const
{
    return "#<array:Explosion>";
}

void
game::interface::ExplosionFunction::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}
