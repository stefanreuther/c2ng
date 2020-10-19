/**
  *  \file game/experiencelevelset.hpp
  *  \brief Typedef game::ExperienceLevelSet_t
  */
#ifndef C2NG_GAME_EXPERIENCELEVELSET_HPP
#define C2NG_GAME_EXPERIENCELEVELSET_HPP

#include "afl/bits/smallset.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "afl/string/translator.hpp"

namespace game {

    /** Set of experience levels.
        Experience levels start at 0. */
    typedef afl::bits::SmallSet<int> ExperienceLevelSet_t;

    /** Format experience level set into string.
        This is intended to format level restrictions, hence it returns an empty string if the set contains all levels.
        Otherwise, returns a human-readable list of the experience levels in \c set.

        It consults host version and configuration for experience limits, so that a "level 0-4" set can be formatted
        as "all levels" if the maximum level is 4.

        \param set Set to format
        \param host Host version
        \param config Host configuration
        \param tx Translator */
    String_t formatExperienceLevelSet(ExperienceLevelSet_t set,
                                      const game::HostVersion& host,
                                      const game::config::HostConfiguration& config,
                                      afl::string::Translator& tx);

}

#endif
