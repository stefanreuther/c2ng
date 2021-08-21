/**
  *  \file game/v3/utils.cpp
  */

#include "game/v3/utils.hpp"
#include "game/v3/structures.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/filesystem.hpp"

namespace gt = game::v3::structures;
using afl::io::FileSystem;

void
game::v3::loadRaceNames(PlayerList& list, afl::io::Directory& dir, afl::charset::Charset& charset)
{
    // ex GRaceNameList::load, ccload.pas:LoadRaces
    list.clear();

    // Load the file
    afl::base::Ref<afl::io::Stream> file = dir.openFile("race.nm", FileSystem::OpenRead);
    gt::RaceNames in;
    file->fullRead(afl::base::fromObject(in));
    for (int player = 0; player < gt::NUM_PLAYERS; ++player) {
        if (Player* out = list.create(player+1)) {
            out->setName(Player::ShortName,     charset.decode(in.shortNames[player]));
            out->setName(Player::LongName,      charset.decode(in.longNames[player]));
            out->setName(Player::AdjectiveName, charset.decode(in.adjectiveNames[player]));
            out->setOriginalNames();
        }
    }

    // Create aliens
    if (Player* aliens = list.create(gt::NUM_PLAYERS+1)) {
        aliens->initAlien();
    }
}

void
game::v3::encryptTarget(game::v3::structures::ShipTarget& target)
{
    for (int i = 0; i < 20; ++i) {
        target.name.m_bytes[i] = uint8_t(target.name.m_bytes[i] ^ (154-i));
    }
}
