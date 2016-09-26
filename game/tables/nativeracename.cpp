/**
  *  \file game/tables/nativeracename.cpp
  */

#include "game/tables/nativeracename.hpp"
#include "util/translation.hpp"
#include "afl/base/countof.hpp"

namespace {
    static const char*const NAMES[] = {
        N_("none"),
        N_("Humanoid"),
        N_("Bovinoid"),
        N_("Reptilian"),
        N_("Avian"),
        N_("Amorphous"),
        N_("Insectoid"),
        N_("Amphibian"),
        N_("Ghipsoldal"),
        N_("Siliconoid"),
        N_("Divine"),           // This and the following are proposed native races.
        N_("Artificial"),
        N_("Spirits"),
        N_("Viral"),
        N_("Plasmaoid"),
        N_("Gaseous"),
    };
    const int MAX_RACE = countof(NAMES)-1;
}

game::tables::NativeRaceName::NativeRaceName(afl::string::Translator& tx)
    : m_translator(tx)
{ }

String_t
game::tables::NativeRaceName::get(int race) const
{
    if (race < 0 || race > MAX_RACE) {
        return "?";
    } else {
        return m_translator.translateString(NAMES[race]);
    }
}

bool
game::tables::NativeRaceName::getFirstKey(int& a) const
{
    a = 0;
    return true;
}

bool
game::tables::NativeRaceName::getNextKey(int& a) const
{
    if (a < MAX_RACE) {
        ++a;
        return true;
    } else {
        return false;
    }
}
