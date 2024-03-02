/**
  *  \file test/game/vcr/databasetest.cpp
  *  \brief Test for game::vcr::Database
  */

#include "game/vcr/database.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.vcr.Database")
{
    class Tester : public game::vcr::Database {
     public:
        virtual size_t getNumBattles() const
            { return 0; }
        virtual game::vcr::Battle* getBattle(size_t /*nr*/)
            { return 0; }
        virtual void save(afl::io::Stream& /*out*/, size_t /*first*/, size_t /*num*/, const game::config::HostConfiguration& /*config*/, afl::charset::Charset& /*cs*/)
            { }
    };
    Tester t;
}
