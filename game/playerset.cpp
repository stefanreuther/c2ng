/**
  *  \file game/playerset.cpp
  *  \brief Type game::PlayerSet_t
  */

#include "game/playerset.hpp"
#include "afl/string/format.hpp"
#include "game/limits.hpp"
#include "game/playerlist.hpp"
#include "util/string.hpp"
#include "util/stringparser.hpp"

// Format set of players.
String_t
game::formatPlayerSet(PlayerSet_t set, const PlayerList& list, afl::string::Translator& tx)
{
    // ex game/playerset.cc:formatPlayerSet, hullfunc.pas::FormatRaceMask
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
                return afl::string::Format(tx("player %d"), i);
            }
        }
    }
    if ((allPlayers - set).isUnitSet()) {
        // all but one player
        for (int i = 0, n = list.size(); i < n; ++i) {
            if (set == allPlayers - i) {
                return afl::string::Format(tx("all but player %d"), i);
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

// Format set of players including host.
String_t
game::formatPlayerHostSet(PlayerSet_t set, const PlayerList& list, afl::string::Translator& tx)
{
    const bool hasHost = set.contains(0);
    const PlayerSet_t allPlayers = (list.getAllPlayers() - 0);
    set &= allPlayers;

    if (set == allPlayers) {
        // everyone
        if (hasHost) {
            return tx.translateString("host, all players");
        } else {
            return tx.translateString("all players");
        }
    }
    if (set.empty()) {
        // nobody
        if (hasHost) {
            return tx.translateString("host");
        } else {
            return tx.translateString("nobody");
        }
    }

    String_t formattedPlayers = formatPlayerSet(set, list, tx);
    if (hasHost) {
        // formattedPlayers can be "all but player X", so put host in front to make it unambiguous
        formattedPlayers = afl::string::Format(tx("host, %s"), formattedPlayers);
    }
    return formattedPlayers;
}

// Format player set into number list ("1 3 5").
String_t
game::formatPlayerSetAsList(PlayerSet_t set)
{
    // ex phost.pas:RaceNumberListToString
    String_t result;
    for (int i = 0; i <= MAX_PLAYERS; ++i) {
        if (set.contains(i)) {
            util::addListItem(result, " ", afl::string::Format("%d", i));
        }
    }
    return result;
}

// Parse number list ("1 3 5") into player set.
game::PlayerSet_t
game::parsePlayerListAsSet(const String_t& str)
{
    // ex phost.pas:ParseRaceNumberList
    PlayerSet_t result;
    util::StringParser p(str);
    while (!p.parseEnd()) {
        int pl;
        if (p.parseInt(pl)) {
            if (pl >= 0 && pl <= MAX_PLAYERS) {
                result += pl;
            }
        } else {
            p.consumeCharacter();
        }
    }
    return result;
}
