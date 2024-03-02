/**
  *  \file game/vcr/test/database.hpp
  *  \brief Class game::vcr::test::Database
  */
#ifndef C2NG_GAME_VCR_TEST_DATABASE_HPP
#define C2NG_GAME_VCR_TEST_DATABASE_HPP

#include "game/vcr/database.hpp"
#include "game/vcr/test/battle.hpp"
#include "afl/container/ptrvector.hpp"

namespace game { namespace vcr { namespace test {

    /** VCR Database for testing.

        Add battles using addBattle(), then configure them using Battle methods. */
    class Database : public game::vcr::Database {
     public:
        /** Constructor.
            Makes an empty database. */
        Database();

        /** Destructor. */
        ~Database();

        /** Add a battle.
            @return Newly-allocated empty battle. */
        Battle& addBattle();

        // Database virtuals:
        virtual size_t getNumBattles() const;
        virtual Battle* getBattle(size_t nr);
        virtual void save(afl::io::Stream& out, size_t first, size_t num, const game::config::HostConfiguration& config, afl::charset::Charset& cs);

     private:
        afl::container::PtrVector<Battle> m_battles;
    };

} } }

#endif
