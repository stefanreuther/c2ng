/**
  *  \file u/t_game_vcr_database.cpp
  *  \brief Test for game::vcr::Database
  */

#include "game/vcr/database.hpp"

#include "t_game_vcr.hpp"

/** Interface test. */
void
TestGameVcrDatabase::testIt()
{
    class Tester : public game::vcr::Database {
     public:
        virtual size_t getNumBattles() const
            { return 0; }
        virtual game::vcr::Battle* getBattle(size_t /*nr*/)
            { return 0; }
    };
    Tester t;
}

