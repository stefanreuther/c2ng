/**
  *  \file server/file/racenames.cpp
  */

#include "server/file/racenames.hpp"
#include "game/v3/structures.hpp"
#include "afl/except/filetooshortexception.hpp"
#include "afl/io/stream.hpp"

void
server::file::loadRaceNames(RaceNames_t& out, afl::base::ConstBytes_t data, afl::charset::Charset& cs)
{
    // Parse
    game::v3::structures::RaceNames in;
    if (data.size() < sizeof(in)) {
        throw afl::except::FileTooShortException("<race.nm>");
    }
    afl::base::fromObject(in).copyFrom(data);

    // Convert
    for (int player = 0; player < game::v3::structures::NUM_PLAYERS; ++player) {
        out.set(player+1, cs.decode(in.longNames[player]));
    }
}

void
server::file::loadRaceNames(RaceNames_t& out, afl::io::Directory& dir, afl::charset::Charset& cs)
{
    afl::base::Ref<afl::io::Stream> s = dir.openFile("race.nm", afl::io::FileSystem::OpenRead);
    uint8_t buffer[sizeof(game::v3::structures::RaceNames)];
    s->fullRead(buffer);
    loadRaceNames(out, buffer, cs);
}
