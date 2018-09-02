/**
  *  \file game/v3/utils.hpp
  */
#ifndef C2NG_GAME_V3_UTILS_HPP
#define C2NG_GAME_V3_UTILS_HPP

#include "afl/charset/charset.hpp"
#include "afl/io/directory.hpp"
#include "game/playerlist.hpp"
#include "game/v3/structures.hpp"

namespace game { namespace v3 {

    void loadRaceNames(PlayerList& list, afl::io::Directory& dir, afl::charset::Charset& charset);

    void encryptTarget(game::v3::structures::ShipTarget& target);

} }

#endif
