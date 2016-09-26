/**
  *  \file game/tables/ionstormclassname.cpp
  */

#include "game/tables/ionstormclassname.hpp"

game::tables::IonStormClassName::IonStormClassName(afl::string::Translator& tx)
    : m_translator(tx)
{ }

String_t
game::tables::IonStormClassName::get(int32_t voltage) const
{
    // ex game/tables.h:getIonStormClassName
    if (voltage < 50) {
        return m_translator.translateString("harmless");
    } else if (voltage < 100) {
        return m_translator.translateString("moderate");
    } else if (voltage < 150) {
        return m_translator.translateString("strong");
    } else if (voltage < 200) {
        return m_translator.translateString("dangerous");
    } else {
        return m_translator.translateString("VERY dangerous");
    }
}

bool
game::tables::IonStormClassName::getFirstKey(int32_t& a) const
{
    a = 0;
    return true;
}

bool
game::tables::IonStormClassName::getNextKey(int32_t& a) const
{
    // Steps of 50, so it's simple
    if (a < 200) {
        a += 50;
        return true;
    } else {
        return false;
    }
}
