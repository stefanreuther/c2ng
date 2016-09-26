/**
  *  \file game/tables/industrylevel.cpp
  */

#include <algorithm>
#include "game/tables/industrylevel.hpp"
#include "util/translation.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/staticassert.hpp"
#include "game/types.hpp"

namespace {
    static const char*const NAMES[] = {
        N_("minimal"),
        N_("light"),
        N_("moderate"),
        N_("substantial"),
        N_("heavy"),
    };
    const int MAX_LEVEL = countof(NAMES)-1;
}

game::tables::IndustryLevel::IndustryLevel(afl::string::Translator& tx)
    : m_translator(tx)
{ }

String_t
game::tables::IndustryLevel::get(int level) const
{
    return m_translator.translateString(NAMES[std::max(0, std::min(MAX_LEVEL, level))]);
}

bool
game::tables::IndustryLevel::getFirstKey(int& a) const
{
    a = 0;
    return true;
}

bool
game::tables::IndustryLevel::getNextKey(int& a) const
{
    static_assert(MinimalIndustry == 0,     "MinimalIndustry");
    static_assert(LightIndustry == 1,       "LightIndustry");
    static_assert(ModerateIndustry == 2,    "ModerateIndustry");
    static_assert(SubstantialIndustry == 3, "SubstantialIndustry");
    static_assert(HeavyIndustry == 4,       "HeavyIndustry");
    if (a < MAX_LEVEL) {
        ++a;
        return true;
    } else {
        return false;
    }
}
