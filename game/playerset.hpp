/**
  *  \file game/playerset.hpp
  *  \brief Type game::PlayerSet_t
  */
#ifndef C2NG_GAME_PLAYERSET_HPP
#define C2NG_GAME_PLAYERSET_HPP

#include "afl/bits/smallset.hpp"
#include "afl/string/translator.hpp"

namespace game {

    class PlayerList;

    /** Set of players.
        Depending on context, slot 0 means "unowned" or "host". */
    typedef afl::bits::SmallSet<int> PlayerSet_t;

    /** Format set of players.
        Formats the list into a nice user-friendly string.
        This function assumes that the set actually contains players only.
        If the set contains all players, the result is an empty string
        (assuming that it's not useful to inform the player about something everyone has).
        \param set   Set to format
        \param list  Player list. This list defines the set of all players.
        \param tx    Translator
        \return result */
    String_t formatPlayerSet(PlayerSet_t set, const PlayerList& list, afl::string::Translator& tx);

    /** Format set of players including host.
        Formats the list into a nice user-friendly string.
        This function assumes that the set contains players, and that slot 0 means host.
        The result is never an empty string.
        \param set   Set to format
        \param list  Player list. This list defines the set of all players.
        \param tx    Translator
        \return result */
    String_t formatPlayerHostSet(PlayerSet_t set, const PlayerList& list, afl::string::Translator& tx);

}

#endif
