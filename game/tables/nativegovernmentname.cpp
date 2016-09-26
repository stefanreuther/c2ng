/**
  *  \file game/tables/nativegovernmentname.cpp
  */

#include "game/tables/nativegovernmentname.hpp"
#include "util/translation.hpp"
#include "afl/base/countof.hpp"

namespace {
    static const char*const NAMES[] = {
        N_("none"),
        N_("Anarchy"),
        N_("Pre-Tribal"),
        N_("Early-Tribal"),
        N_("Tribal"),
        N_("Feudal"),
        N_("Monarchy"),
        N_("Representative"),
        N_("Participatory"),
        N_("Unity")
    };
    const int MAX_GOVERNMENT = countof(NAMES)-1;
}

game::tables::NativeGovernmentName::NativeGovernmentName(afl::string::Translator& tx)
    : m_translator(tx)
{ }

String_t
game::tables::NativeGovernmentName::get(int gov) const
{
    if (gov < 0 || gov > MAX_GOVERNMENT) {
        return "?";
    } else {
        return m_translator.translateString(NAMES[gov]);
    }
}

bool
game::tables::NativeGovernmentName::getFirstKey(int& a) const
{
    a = 0;
    return true;
}

bool
game::tables::NativeGovernmentName::getNextKey(int& a) const
{
    if (a < MAX_GOVERNMENT) {
        ++a;
        return true;
    } else {
        return false;
    }
}
