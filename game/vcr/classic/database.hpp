/**
  *  \file game/vcr/classic/database.hpp
  *  \brief Class game::vcr::classic::Database
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

    /** Classic VCR database.
        Implements the game::vcr::Database interface for 1:1 combat. */
    class Database : public game::vcr::Database {
     public:
        /** Constructor.
            Makes an empty database. */
        Database();

        /** Destructor. */
        ~Database();

        /** Load from file.
            Assumes this Database to be empty.
            Load a VCR.DAT/VCR.HST file.
            Recognizes (and ignores) special hacks: PHost 2's configuration battle, dummy battle added by CORR.

            @param file    File
            @param config  Host configuration (for decoding PlanetsHaveTubes)
            @parma charset Game character set (for decoding names) */
        void load(afl::io::Stream& file, const game::config::HostConfiguration& config, afl::charset::Charset& charset);

        /** Add a newly-constructed battle.
            @param battle Battle; must not be null
            @return battle */
        Battle* addNewBattle(Battle* battle);

        // game::vcr::Database methods:
        virtual size_t getNumBattles() const;
        virtual Battle* getBattle(size_t nr);

     private:
        afl::container::PtrVector<Battle> m_battles;
    };

} } }

#endif
