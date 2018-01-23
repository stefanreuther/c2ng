/**
  *  \file game/db/fleetloader.hpp
  */
#ifndef C2NG_GAME_DB_FLEETLOADER_HPP
#define C2NG_GAME_DB_FLEETLOADER_HPP

#include "game/map/universe.hpp"
#include "afl/io/directory.hpp"
#include "afl/charset/charset.hpp"

namespace game { namespace db {

    class FleetLoader {
     public:
        FleetLoader(afl::charset::Charset& cs);

        void load(afl::io::Directory& dir, game::map::Universe& univ, int playerNumber);

     private:
        afl::charset::Charset& m_charset;
    };

} }

#endif
