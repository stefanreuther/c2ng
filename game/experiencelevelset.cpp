/**
  *  \file game/experiencelevelset.cpp
  */

#include "game/experiencelevelset.hpp"
#include "game/limits.hpp"
#include "afl/string/format.hpp"

String_t
game::formatExperienceLevelSet(ExperienceLevelSet_t set,
                               const game::HostVersion& host,
                               const game::config::HostConfiguration& config,
                               afl::string::Translator& tx)
{
    // ex game/exp.cc:formatExpLevelSet
    ExperienceLevelSet_t allLevels = host.hasExperienceLevels() ? ExperienceLevelSet_t::allUpTo(config[config.NumExperienceLevels]()) : ExperienceLevelSet_t(0);
    set &= allLevels;

    if (set == allLevels) {
        // all levels
        return String_t();
    }
    if (set.empty()) {
        // no levels
        return tx.translateString("no level");
    }
    if (set.isUnitSet()) {
        // one level
        for (int i = 0; i <= MAX_EXPERIENCE_LEVELS; ++i) {
            if (set.contains(i)) {
                return afl::string::Format(tx.translateString("level %d").c_str(), i);
            }
        }
    }

    // Find minimum level
    int minLevel = 0;
    while (!set.contains(minLevel)) {
        ++minLevel;
    }

    if (minLevel != 0 && set + ExperienceLevelSet_t::allUpTo(minLevel-1) == allLevels) {
        // all levels from X onwards
        return afl::string::Format(tx.translateString("level %d+").c_str(), minLevel);
    }

    // mixed
    String_t result;
    String_t prefix = tx.translateString("levels ");
    for (int i = 0; i <= MAX_EXPERIENCE_LEVELS; ++i) {
        if (set.contains(i)) {
            result += prefix;
            result += afl::string::Format("%d", i);
            prefix = ", ";
        }
    }
    return result;
}
