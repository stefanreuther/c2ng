/**
  *  \file server/file/racenames.hpp
  */
#ifndef C2NG_SERVER_FILE_RACENAMES_HPP
#define C2NG_SERVER_FILE_RACENAMES_HPP

#include "game/playerarray.hpp"
#include "afl/string/string.hpp"
#include "afl/base/memory.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/directory.hpp"

namespace server { namespace file {

    typedef game::PlayerArray<String_t> RaceNames_t;

    void loadRaceNames(RaceNames_t& out, afl::base::ConstBytes_t data, afl::charset::Charset& cs);
    void loadRaceNames(RaceNames_t& out, afl::io::Directory& dir, afl::charset::Charset& cs);

} }

#endif
