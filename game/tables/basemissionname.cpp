/**
  *  \file game/tables/basemissionname.cpp
  */

#include "game/tables/basemissionname.hpp"

game::tables::BaseMissionName::BaseMissionName(afl::string::Translator& tx)
    : m_translator(tx)
{ }

// /** Get starbase mission name.
//     \param mission mission number
//     \return statically-allocated, translated string. */
String_t
game::tables::BaseMissionName::get(int mission) const
{
    // ex game/tables.h:getBaseMissionName
    switch (mission) {
     case 0:
        return m_translator.translateString("none");
     case 1:
        return m_translator.translateString("Refuel ships");
     case 2:
        return m_translator.translateString("Maximize defense");
     case 3:
        return m_translator.translateString("Load torpedoes onto ships");
     case 4:
        return m_translator.translateString("Unload incoming ships");
     case 5:
        return m_translator.translateString("Repair base");
     case 6:
        return m_translator.translateString("Force surrender");
     default:
        return "?";
    }
}

bool
game::tables::BaseMissionName::getFirstKey(int& a) const
{
    a = 0;
    return true;
}

bool
game::tables::BaseMissionName::getNextKey(int& a) const
{
    if (a < 6) {
        ++a;
        return true;
    } else {
        return false;
    }
}
