/**
  *  \file game/vcr/classic/utils.cpp
  *  \brief Classic VCR Utilities
  */

#include "game/vcr/classic/utils.hpp"
#include "afl/string/format.hpp"

using afl::string::Format;

String_t
game::vcr::classic::formatBattleResult(BattleResult_t result,
                                       const String_t& leftName, TeamSettings::Relation leftRelation,
                                       const String_t& rightName, TeamSettings::Relation rightRelation,
                                       const String_t& annotation,
                                       afl::string::Translator& tx)
{
    int me;
    if (leftRelation == TeamSettings::ThisPlayer) {
        me = 0;
    } else if (rightRelation == TeamSettings::ThisPlayer) {
        me = 1;
    } else {
        me = 2;
    }

    // Build annotation
    String_t insert;
    if (!annotation.empty()) {
        insert = Format(" (%s)", annotation);
    }

    String_t s;
    if (result.empty()) {
        s = tx("unknown. Wait while computing...");
    } else if (result == Invalid) {
        s = tx("Battle cannot be played!");
    } else if (result == Timeout) {
        s = tx("Battle timed out (too long).");
    } else if (result == Stalemate) {
        s = tx("Stalemate.");
    } else if (result == LeftDestroyed) {
        if (me == 0) {
            s = Format(tx("We were destroyed%s.").c_str(), insert);
        } else if (me == 1) {
            s = Format(tx("We won%s.").c_str(), insert);
        } else {
            s = Format(tx("%s won%s.").c_str(), rightName, insert);
        }
    } else if (result == RightDestroyed) {
        if (me == 1) {
            s = Format(tx("We were destroyed%s.").c_str(), insert);
        } else if (me == 0) {
            s = Format(tx("We won%s.").c_str(), insert);
        } else {
            s = Format(tx("%s won%s.").c_str(), leftName, insert);
        }
    } else if (result == LeftCaptured) {
        if (me == 0) {
            s = Format(tx("They have captured our ship%s.").c_str(), insert);
        } else if (me == 1) {
            s = Format(tx("We captured their ship%s.").c_str(), insert);
        } else {
            s = Format(tx("%s was captured%s.").c_str(), leftName, insert);
        }
    } else if (result == RightCaptured) {
        if (me == 1) {
            s = Format(tx("They have captured our ship%s.").c_str(), insert);
        } else if (me == 0) {
            s = Format(tx("We captured their ship%s.").c_str(), insert);
        } else {
            s = Format(tx("%s was captured%s.").c_str(), rightName, insert);
        }
    } else if (result == BattleResult_t(LeftDestroyed) + RightDestroyed) {
        s = tx("Both were destroyed.");
    } else {
        s = tx("Both are disabled.");
    }
    return s;
}
