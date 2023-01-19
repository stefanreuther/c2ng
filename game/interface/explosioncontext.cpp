/**
  *  \file game/interface/explosioncontext.cpp
  */

#include "game/interface/explosioncontext.hpp"
#include "interpreter/typehint.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "game/interface/explosionproperty.hpp"
#include "game/game.hpp"

namespace {
    enum ExplosionDomain {
        ExplosionPropertyDomain
    };

    const interpreter::NameTable EXPLOSION_MAP[] = {
        { "ID",         game::interface::iepId,       ExplosionPropertyDomain, interpreter::thInt },
        { "ID.SHIP",    game::interface::iepShipId,   ExplosionPropertyDomain, interpreter::thInt },
        { "LOC.X",      game::interface::iepLocX,     ExplosionPropertyDomain, interpreter::thInt },
        { "LOC.Y",      game::interface::iepLocY,     ExplosionPropertyDomain, interpreter::thInt },
        { "NAME",       game::interface::iepName,     ExplosionPropertyDomain, interpreter::thString },
        { "NAME.SHIP",  game::interface::iepShipName, ExplosionPropertyDomain, interpreter::thString },
        { "TYPE",       game::interface::iepTypeStr,  ExplosionPropertyDomain, interpreter::thString },
        { "TYPE.SHORT", game::interface::iepTypeChar, ExplosionPropertyDomain, interpreter::thString },
    };
}

game::interface::ExplosionContext::ExplosionContext(Id_t id, Session& session, afl::base::Ref<Turn> turn)
    : m_id(id),
      m_session(session),
      m_turn(turn)
{ }

game::interface::ExplosionContext::~ExplosionContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::ExplosionContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    return lookupName(name, EXPLOSION_MAP, result) ? this : 0;
}

afl::data::Value*
game::interface::ExplosionContext::get(PropertyIndex_t index)
{
    if (game::map::Explosion* expl = getObject()) {
        switch (ExplosionDomain(EXPLOSION_MAP[index].domain)) {
         case ExplosionPropertyDomain:
            return getExplosionProperty(*expl, ExplosionProperty(EXPLOSION_MAP[index].index), m_session.translator(), m_session.interface());
        }
        return 0;
    } else {
        return 0;
    }
}

bool
game::interface::ExplosionContext::next()
{
    if (Id_t nextId = m_turn->universe().explosions().findNextIndexNoWrap(m_id, false)) {
        m_id = nextId;
        return true;
    } else {
        return false;
    }
}

game::interface::ExplosionContext*
game::interface::ExplosionContext::clone() const
{
    return new ExplosionContext(m_id, m_session, m_turn);
}

game::map::Explosion*
game::interface::ExplosionContext::getObject()
{
    return m_turn->universe().explosions().getObjectByIndex(m_id);
}

void
game::interface::ExplosionContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(EXPLOSION_MAP);
}

// BaseValue:
String_t
game::interface::ExplosionContext::toString(bool /*readable*/) const
{
    return "#<explosion>";
}

void
game::interface::ExplosionContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

game::interface::ExplosionContext*
game::interface::ExplosionContext::create(Id_t id, Session& session)
{
    Game* g = session.getGame().get();
    if (g == 0) {
        return 0;
    }
    Turn& t = g->currentTurn();

    game::map::Explosion* expl = t.universe().explosions().getObjectByIndex(id);
    if (expl == 0) {
        return 0;
    }

    return new ExplosionContext(id, session, t);
}
