/**
  *  \file game/turnloader.cpp
  */

#include "game/turnloader.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/limits.hpp"

int
game::TurnLoader::getDefaultPlayer(PlayerSet_t baseSet) const
{
    // We don't care for the string
    afl::string::NullTranslator tx;
    String_t tmp;

    PlayerStatus foundStatus = Available;
    int foundPlayer = 0;
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        if (baseSet.contains(i)) {
            PlayerStatusSet_t thisSet = getPlayerStatus(i, tmp, tx);
            if (thisSet.contains(Available)) {
                PlayerStatus thisStatus = thisSet.contains(Primary) ? Primary : thisSet.contains(Playable) ? Playable : Available;
                if (thisStatus > foundStatus || foundPlayer == 0) {
                    // Better status or first finding: keep it
                    foundPlayer = i;
                    foundStatus = thisStatus;
                } else if (thisStatus == foundStatus && foundPlayer != 0) {
                    // Same status and second finding: ambiguous
                    foundPlayer = -1;
                } else {
                    // Worse status: skip
                }
            }
        }
    }
    return foundPlayer > 0 ? foundPlayer : 0;
}
