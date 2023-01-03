/**
  *  \file game/vcr/classic/utils.hpp
  *  \brief Classic VCR Utilities
  */
#ifndef C2NG_GAME_VCR_CLASSIC_UTILS_HPP
#define C2NG_GAME_VCR_CLASSIC_UTILS_HPP

#include "afl/string/translator.hpp"
#include "game/teamsettings.hpp"
#include "game/vcr/classic/types.hpp"

namespace game { namespace vcr { namespace classic {

    /** Describe a battle result.
        @param result        Result
        @param leftName      Name of left unit
        @param leftRelation  Our relation to left unit
        @param rightName     Name of right unit
        @param rightRelation Our relation to right unit
        @param annotation    Additional annotation; will be included in parentheses if the description refers to either side
        @param tx            Translator
        @return Formatted battle result ("We won.") */
    String_t formatBattleResult(BattleResult_t result,
                                const String_t& leftName, TeamSettings::Relation leftRelation,
                                const String_t& rightName, TeamSettings::Relation rightRelation,
                                const String_t& annotation,
                                afl::string::Translator& tx);

} } }

#endif
