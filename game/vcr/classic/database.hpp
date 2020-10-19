/**
  *  \file game/vcr/classic/database.hpp
  */
#ifndef C2NG_GAME_VCR_CLASSIC_DATABASE_HPP
#define C2NG_GAME_VCR_CLASSIC_DATABASE_HPP

#include "afl/charset/charset.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/stream.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/vcr/classic/battle.hpp"
#include "game/vcr/database.hpp"

namespace game { namespace vcr { namespace classic {

    class Database : public game::vcr::Database {
     public:
        Database();

        ~Database();

        void load(afl::io::Stream& file,
                  const game::config::HostConfiguration& config,
                  afl::charset::Charset& charset);

        Battle* addNewBattle(Battle* battle);

        // game::vcr::Database methods:
        virtual size_t getNumBattles() const;
        virtual Battle* getBattle(size_t nr);

     private:
        afl::container::PtrVector<Battle> m_battles;
    };

} } }

#endif
