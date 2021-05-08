/**
  *  \file game/tables/happinesschangename.cpp
  */

#include "game/tables/happinesschangename.hpp"

game::tables::HappinessChangeName::HappinessChangeName(afl::string::Translator& tx)
    : m_translator(tx)
{ }

String_t
game::tables::HappinessChangeName::get(int change) const
{
    // ex game/tables.h:getHappyChangeName
    // ex planint.pas:ChangeStr
    if (change < -5) {
        return m_translator("They HATE you!");
    } else if (change < 0) {
        return m_translator("They are angry about you!");
    } else if (change == 0) {
        return m_translator("They are undecided about you.");
    } else if (change > 4) {
        return m_translator("They LOVE you.");
    } else {
        return m_translator("They like your leadership.");
    }
}

bool
game::tables::HappinessChangeName::getFirstKey(int& a) const
{
    a = -10;
    return true;
}

bool
game::tables::HappinessChangeName::getNextKey(int& a) const
{
    if (a < -5) {
        a = -5;
        return true;
    } else if (a < 0) {
        a = 0;
        return true;
    } else if (a == 0) {
        a = 1;
        return true;
    } else if (a > 4) {
        return false;
    } else {
        a = 5;
        return true;
    }
}

