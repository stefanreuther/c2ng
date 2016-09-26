/**
  *  \file game/playerset.cpp
  */

#include "game/playerset.hpp"
#include "afl/string/format.hpp"
#include "game/playerlist.hpp"

// /** Format player set to nice human-readable string. */
String_t
game::formatPlayerSet(PlayerSet_t set, const PlayerList& list, afl::string::Translator& tx)
{
    // game/playerset.cc:formatPlayerSet
    // hullfunc.pas::FormatRaceMask
    const PlayerSet_t allPlayers = list.getAllPlayers();
    set &= allPlayers;
    if (set == allPlayers) {
        // all players -- nothing to mention
        return String_t();
    }
    if (set.empty()) {
        // no player
        return tx.translateString("nobody");
    }
    if (set.isUnitSet()) {
        // one player
        for (int i = 0, n = list.size(); i < n; ++i) {
            if (set == PlayerSet_t(i)) {
                return afl::string::Format(tx.translateString("player %d").c_str(), i);
            }
        }
    }
    if ((allPlayers - set).isUnitSet()) {
        // all but one player
        for (int i = 0, n = list.size(); i < n; ++i) {
            if (set == allPlayers - i) {
                return afl::string::Format(tx.translateString("all but player %d").c_str(), i);
            }
        }
    }

    // generic version
    String_t result;
    String_t prefix = tx.translateString("players ");
    for (int i = 0, n = list.size(); i < n; ++i) {
        if (set.contains(i)) {
            result += prefix;
            result += afl::string::Format("%d", i);
            prefix = ", ";
        }
    }
    return result;
}
