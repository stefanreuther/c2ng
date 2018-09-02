/**
  *  \file game/ref/sortbyname.cpp
  */

#include "game/ref/sortbyname.hpp"
#include "game/turn.hpp"
#include "game/game.hpp"

game::ref::SortByName::SortByName(Session& session)
    : m_session(session)
{ }

int
game::ref::SortByName::compare(const Reference& a, const Reference& b) const
{
    // ex sortByName
    return afl::string::strCaseCompare(getName(a), getName(b));
}

String_t
game::ref::SortByName::getClass(const Reference& /*a*/) const
{
    return String_t();
}

String_t
game::ref::SortByName::getName(const Reference& a) const
{
    // Try to resolve as plain object name
    // FIXME: make this a function of Session?
    if (Game* g = m_session.getGame().get()) {
        Turn* t = g->getViewpointTurn().get();
        if (t == 0) {
            t = &g->currentTurn();
        }
        if (const game::map::Object* obj = t->universe().getObject(a)) {
            String_t name = obj->getName(game::map::Object::PlainName, m_session.translator(), m_session.interface());
            if (!name.empty()) {
                return name;
            }
        }
    }

    // Resolve as reference name
    String_t name;
    if (!m_session.getReferenceName(a, name)) {
        name = a.toString(m_session.translator());
    }
    return name;
}
