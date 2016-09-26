/**
  *  \file game/playerset.hpp
  */
#ifndef C2NG_GAME_PLAYERSET_HPP
#define C2NG_GAME_PLAYERSET_HPP

#include "afl/bits/smallset.hpp"
#include "afl/string/translator.hpp"

namespace game {

    class PlayerList;

    typedef afl::bits::SmallSet<int> PlayerSet_t;

    String_t formatPlayerSet(PlayerSet_t set, const PlayerList& list, afl::string::Translator& tx);

}

#endif
