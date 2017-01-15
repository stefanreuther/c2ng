/**
  *  \file game/maint/sweeper.hpp
  */
#ifndef C2NG_GAME_MAINT_SWEEPER_HPP
#define C2NG_GAME_MAINT_SWEEPER_HPP

#include "afl/io/directory.hpp"
#include "game/playerset.hpp"

namespace game { namespace maint {

    class Sweeper {
     public:
        Sweeper();

        void scan(afl::io::Directory& dir);
        void execute(afl::io::Directory& dir);

        void setEraseDatabase(bool flag);
        void setPlayers(PlayerSet_t set);
        PlayerSet_t getPlayers() const;

        PlayerSet_t getRemainingPlayers();

     private:
        bool m_eraseDatabaseFlag;
        bool m_didScan;

        PlayerSet_t m_remainingPlayers;
        PlayerSet_t m_selectedPlayers;

        void processPlayerFiles(afl::io::Directory& dir, int player);
        void updateIndex(afl::io::Directory& dir);
    };

} }

#endif
