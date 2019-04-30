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
    // Resolve as plain name; if that does not work, as reference name.
    String_t name;
    if (!m_session.getReferenceName(a, PlainName, name)) {
        name = a.toString(m_session.translator());
    }
    return name;
}
