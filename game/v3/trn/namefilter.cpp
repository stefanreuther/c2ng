/**
  *  \file game/v3/trn/namefilter.cpp
  */

#include "game/v3/trn/namefilter.hpp"

game::v3::trn::NameFilter::NameFilter(String_t name, bool wildcard)
    : Filter(),
      m_name(name),
      m_wildcard(wildcard)
{ }

bool
game::v3::trn::NameFilter::accept(const TurnFile& trn, size_t index) const
{
    // ex FilterName::accept
    if (const char* commandName = trn.getCommandName(index)) {
        String_t s(commandName);
        if (m_wildcard && s.length() > m_name.length()) {
            s.erase(m_name.length());
        }
        if (afl::string::strCaseCompare(s, m_name) == 0) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}
