/**
  *  \file game/experiencelevelset.hpp
  */
#ifndef C2NG_GAME_EXPERIENCELEVELSET_HPP
#define C2NG_GAME_EXPERIENCELEVELSET_HPP

#include "afl/bits/smallset.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "afl/string/translator.hpp"

namespace game {

    typedef afl::bits::SmallSet<int> ExperienceLevelSet_t;

    /** Format experience level set into string.
        This is intended to format level restrictions, hence it returns an empty string if the set contains all levels.
        Otherwise, returns a human-readable list of the experience levels in \c set. */
    String_t formatExperienceLevelSet(ExperienceLevelSet_t set,
                                      const game::HostVersion& host,
                                      const game::config::HostConfiguration& config,
                                      afl::string::Translator& tx);

}

#endif
