/**
  *  \file game/tables/mineraldensityclassname.cpp
  */

#include "game/tables/mineraldensityclassname.hpp"

game::tables::MineralDensityClassName::MineralDensityClassName(afl::string::Translator& tx)
    : m_translator(tx)
{ }

String_t
game::tables::MineralDensityClassName::get(int32_t density) const
{
    // ex game/tables.h:getMineralDensityClassName
    if (density < 10) {
        return m_translator.translateString("very scattered");
    } else if (density < 30) {
        return m_translator.translateString("scattered");
    } else if (density < 40) {
        return m_translator.translateString("dispersed");
    } else if (density < 70) {
        return m_translator.translateString("concentrated");
    } else {
        return m_translator.translateString("large masses");
    }
}

bool
game::tables::MineralDensityClassName::getFirstKey(int32_t& a) const
{
    a = 0;
    return true;
}

bool
game::tables::MineralDensityClassName::getNextKey(int32_t& a) const
{
    if (a < 10) {
        a = 10;
        return true;
    } else if (a < 30) {
        a = 30;
        return true;
    } else if (a < 40) {
        a = 40;
        return true;
    } else if (a < 70) {
        a = 70;
        return true;
    } else {
        return false;
    }
}
