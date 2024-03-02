/**
  *  \file game/vcr/database.hpp
  *  \brief Base class game::vcr::Database
  */
#ifndef C2NG_GAME_VCR_DATABASE_HPP
#define C2NG_GAME_VCR_DATABASE_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/base/types.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"
#include "game/config/hostconfiguration.hpp"

namespace game { namespace vcr {

    class Battle;

    /** Basic VCR database.
        A VCR database contains (owns) a list of battles. */
    class Database : public afl::base::Deletable, public afl::base::RefCounted {
     public:
        /** Get number of battles in this database. */
        virtual size_t getNumBattles() const = 0;

        /** Get a VCR entry.
            \param nr Number, [0,getNumBattles()) */
        virtual Battle* getBattle(size_t nr) = 0;

        /** Save VCR in binary format.
            \param out    Stream
            \param first  First battle to save
            \param num    Number of battles to save
            \param config Host configuration
            \param cs     Character set */
        virtual void save(afl::io::Stream& out, size_t first, size_t num, const game::config::HostConfiguration& config, afl::charset::Charset& cs) = 0;
    };

} }

#endif
