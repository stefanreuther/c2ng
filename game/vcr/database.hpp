/**
  *  \file game/vcr/database.hpp
  *  \brief Base class game::vcr::Database
  */
#ifndef C2NG_GAME_VCR_DATABASE_HPP
#define C2NG_GAME_VCR_DATABASE_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"

namespace game { namespace vcr {

    class Battle;

    /** Basic VCR database.
        A VCR database contains (owns) a list of battles. */
    class Database : public afl::base::Deletable {
     public:
        /** Get number of battles in this database. */
        virtual size_t getNumBattles() const = 0;

        /** Get a VCR entry.
            \param nr Number, [0,getNumBattles()) */
        virtual Battle* getBattle(size_t nr) = 0;
    };

} }

#endif
