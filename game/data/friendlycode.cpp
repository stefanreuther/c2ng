/**
  *  \file game/data/friendlycode.cpp
  */

#include "game/data/friendlycode.hpp"

game::data::FriendlyCode::FriendlyCode(const String_t& code, const String_t& description)
    : m_code(code),
      m_description(description)
{ }

const String_t&
game::data::FriendlyCode::getCode() const
{
    return m_code;
}

const String_t&
game::data::FriendlyCode::getDescription() const
{
    return m_description;
}

void
game::data::packFriendlyCodeList(FriendlyCodeList_t& out, const game::spec::FriendlyCodeList& in, const PlayerList& players)
{
    for (game::spec::FriendlyCodeList::Iterator_t it = in.begin(), e = in.end(); it != e; ++it) {
        if (const game::spec::FriendlyCode* p = *it) {
            out.push_back(FriendlyCode(p->getCode(), p->getDescription(players)));
        }
    }
}
