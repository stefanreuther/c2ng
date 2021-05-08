/**
  *  \file game/tables/temperaturename.cpp
  */

#include "game/tables/temperaturename.hpp"

game::tables::TemperatureName::TemperatureName(afl::string::Translator& tx)
    : m_translator(tx)
{ }

String_t
game::tables::TemperatureName::get(int temp) const
{
    // ex game/tables.h:getTemperatureName
    // ex planint.pas:TempStr
    if (temp <= 14) {
        return m_translator("arctic");
    } else if (temp <= 39) {
        return m_translator("cool");
    } else if (temp <= 64) {
        return m_translator("warm");
    } else if (temp <= 84) {
        return m_translator("tropical");
    } else {
        return m_translator("desert");
    }
}

bool
game::tables::TemperatureName::getFirstKey(int& a) const
{
    a = 0;
    return true;
}

bool
game::tables::TemperatureName::getNextKey(int& a) const
{
    if (a <= 14) {
        a = 15;
        return true;
    } else if (a <= 39) {
        a = 40;
        return true;
    } else if (a <= 64) {
        a = 65;
        return true;
    } else if (a <= 84) {
        a = 85;
        return true;
    } else {
        return false;
    }
}
