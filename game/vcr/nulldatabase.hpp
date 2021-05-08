/**
  *  \file game/vcr/nulldatabase.hpp
  *  \brief Class game::vcr::NullDatabase
  */
#ifndef C2NG_GAME_VCR_NULLDATABASE_HPP
#define C2NG_GAME_VCR_NULLDATABASE_HPP

#include "game/vcr/database.hpp"

namespace game { namespace vcr {

    /** Null database.
        Contains no battles. */
    class NullDatabase : public Database {
     public:
        virtual size_t getNumBattles() const;
        virtual Battle* getBattle(size_t nr);
    };

} }

#endif
