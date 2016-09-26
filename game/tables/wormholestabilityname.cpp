/**
  *  \file game/tables/wormholestabilityname.cpp
  */

#include "game/tables/wormholestabilityname.hpp"

game::tables::WormholeStabilityName::WormholeStabilityName(afl::string::Translator& tx)
    : m_translator(tx)
{ }

String_t
game::tables::WormholeStabilityName::get(int32_t stab) const
{
    // ex game/tables.h:getWormholeStabilityName
    if (stab <= 0) {
        return m_translator.translateString("very stable (<5%)");
    } else if (stab <= 1) {
        return m_translator.translateString("stable (<15%)");
    } else if (stab <= 2) {
        return m_translator.translateString("mostly stable (<30%)");
    } else if (stab <= 3) {
        return m_translator.translateString("unstable (<50%)");
    } else if (stab <= 4) {
        return m_translator.translateString("very unstable (<80%)");
    } else {
        return m_translator.translateString("completely unstable");
    }
}

bool
game::tables::WormholeStabilityName::getFirstKey(int32_t& a) const
{
    a = 0;
    return true;
}

bool
game::tables::WormholeStabilityName::getNextKey(int32_t& a) const
{
    if (a < 5) {
        ++a;
        return true;
    } else {
        return false;
    }
}
