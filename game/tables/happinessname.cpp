/**
  *  \file game/tables/happinessname.cpp
  */

#include "game/tables/happinessname.hpp"
#include "util/translation.hpp"

game::tables::HappinessName::HappinessName(afl::string::Translator& tx)
    : m_translator(tx)
{ }

String_t
game::tables::HappinessName::get(int happy) const
{
    // ex game/tables.h:getHappinessName
    if (happy >= 90) {
        return m_translator.translateString("happy");
    } else if (happy >= 70) {
        return m_translator.translateString("calm");
    } else if (happy >= 50) {
        return m_translator.translateString("unhappy");
    } else if (happy >= 40) {
        return m_translator.translateString("very angry");
    } else if (happy >= 20) {
        return m_translator.translateString("rioting");
    } else {
        return m_translator.translateString("fighting");
    }
}

bool
game::tables::HappinessName::getFirstKey(int& a) const
{
    a = 90;
    return true;
}

bool
game::tables::HappinessName::getNextKey(int& a) const
{
    if (a >= 90) {
        a = 70;
        return true;
    } else if (a >= 70) {
        a = 50;
        return true;
    } else if (a >= 50) {
        a = 40;
        return true;
    } else if (a >= 40) {
        a = 20;
        return true;
    } else if (a >= 20) {
        a = 0;
        return true;
    } else {
        return false;
    }
}
