/**
  *  \file game/vcr/flak/database.hpp
  *  \brief Class game::vcr::flak::Database
  */
#ifndef C2NG_GAME_VCR_FLAK_DATABASE_HPP
#define C2NG_GAME_VCR_FLAK_DATABASE_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/io/stream.hpp"
#include "game/timestamp.hpp"
#include "game/vcr/database.hpp"
#include "game/vcr/flak/battle.hpp"

namespace game { namespace vcr { namespace flak {

    /** Implementation of VCR database for FLAK. */
    class Database : public game::vcr::Database {
     public:
        Database();
        ~Database();

        /** Load from file.
            \param file     File to read
            \param charset  Game character set
            \param tx       Translator (for error messages)
            \throw afl::except::FileFormatException, afl::except::FileProblemException */
        void load(afl::io::Stream& file, afl::charset::Charset& charset, afl::string::Translator& tx);

        /** Get timestamp of last read file.
            \return timestamp */
        Timestamp getTimestamp() const;

        /** Add newly-allocated battle.
            \param battle FLAK battle, must not be null
            \return battle */
        Battle* addNewBattle(Battle* battle);

        // game::vcr::Database methods:
        virtual size_t getNumBattles() const;
        virtual Battle* getBattle(size_t nr);
        virtual void save(afl::io::Stream& out, size_t first, size_t num, const game::config::HostConfiguration& config, afl::charset::Charset& cs);

     private:
        afl::container::PtrVector<Battle> m_battles;
        Timestamp m_timestamp;

        static Battle* readOneBattle(afl::io::Stream& file, afl::charset::Charset& charset, afl::string::Translator& tx);
    };

} } }

#endif
