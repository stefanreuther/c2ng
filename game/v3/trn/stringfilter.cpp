/**
  *  \file game/v3/trn/stringfilter.cpp
  */

#include "game/v3/trn/stringfilter.hpp"

game::v3::trn::StringFilter::StringFilter(const String_t& str)
    : Filter(),
      m_string(afl::string::strUCase(str))
{ }

bool
game::v3::trn::StringFilter::accept(const TurnFile& trn, size_t index) const
{
    // ex FilterString::accept
    TurnFile::CommandCode_t cmdCode;
    if (!trn.getCommandCode(index, cmdCode)) {
        return false;
    }
    
    String_t text;
    switch (cmdCode) {
     case tcm_ShipChangeFc:
     case tcm_PlanetChangeFc:
        text = afl::string::fromBytes(trn.getCommandData(index).subrange(0, 3));
        break;
     case tcm_ShipChangeName:
        text = afl::string::fromBytes(trn.getCommandData(index).subrange(0, 20));
        break;
     case tcm_SendMessage:
        int id;
        if (trn.getCommandId(index, id)) {
            text = afl::string::fromBytes(trn.getCommandData(index).subrange(0, id));
            for (size_t i = 0, n = text.size(); i < n; ++i) {
                // quick&dirty decoder. We don't need the full decoder here.
                text[i] -= 13;
            }
        }
        break;
     default:
        return false;
    }

    if (m_string.empty()) {
        return true;
    } else {
        return afl::string::strUCase(trn.charset().decode(afl::string::toMemory(text))).find(m_string) != String_t::npos;
    }
}
