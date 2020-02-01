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
    // ex planint.pas:SBMissionName
    switch (mission) {
     case 0:
        return m_translator("none");
     case 1:
        return m_translator("Refuel ships");
     case 2:
        return m_translator("Maximize defense");
     case 3:
        return m_translator("Load torpedoes onto ships");
     case 4:
        return m_translator("Unload incoming ships");
     case 5:
        return m_translator("Repair base");
     case 6:
        return m_translator("Force surrender");
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
