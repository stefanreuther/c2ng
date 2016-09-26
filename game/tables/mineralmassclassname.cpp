/**
  *  \file game/tables/mineralmassclassname.cpp
  */

#include "game/tables/mineralmassclassname.hpp"

game::tables::MineralMassClassName::MineralMassClassName(afl::string::Translator& tx)
    : m_translator(tx)
{ }

String_t
game::tables::MineralMassClassName::get(int32_t mass) const
{
    // ex game/tables.h:getMineralMassClassName
    if (mass >= 5000) {
        return m_translator.translateString("abundant");
    } else if (mass >= 1200) {
        return m_translator.translateString("very common");
    } else if (mass >= 600) {
        return m_translator.translateString("common");
    } else if (mass >= 100) {
        return m_translator.translateString("rare");
    } else if (mass > 0) {
        return m_translator.translateString("very rare");
    } else {
        return m_translator.translateString("none");
    }
}

bool
game::tables::MineralMassClassName::getFirstKey(int32_t& a) const
{
    a = 5000;
    return true;
}

bool
game::tables::MineralMassClassName::getNextKey(int32_t& a) const
{
    if (a >= 5000) {
        a = 1200;
        return true;
    } else if (a >= 1200) {
        a = 600;
        return true;
    } else if (a >= 600) {
        a = 100;
        return true;
    } else if (a >= 100) {
        a = 1;
        return true;
    } else if (a > 0) {
        a = 0;
        return true;
    } else {
        return false;
    }
}
