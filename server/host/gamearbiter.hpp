/**
  *  \file server/host/gamearbiter.hpp
  *  \brief Class server::host::GameArbiter
  */
#ifndef C2NG_SERVER_HOST_GAMEARBITER_HPP
#define C2NG_SERVER_HOST_GAMEARBITER_HPP

#include <set>
#include "afl/base/uncopyable.hpp"
#include "afl/base/types.hpp"

namespace server { namespace host {

    /** Arbiter for access to games.
        Provides a means of synchronizing multiple execution paths that may access a game.

        We currently have two execution paths:
        - main command queue
        - scheduler

        While the main command queue is already serialized implicitly, host may take a while to run.
        During host run, some commands are accepted (such as fetching data),
        others are rejected (such as uploading a turn file).

        GameArbiter manages the list of currently locked games. */
    class GameArbiter {
     public:
        enum Intent {
            /** Simple access to game.
                These accesses do not conflict with a running host. */
            // ex NoIntent/IntentDB
            Simple,

            /** Critical access to game.
                These accesses do conflict with a running host. */
            // ex IntentDB/IntentCheckturn
            Critical,

            /** Running host.
                Game is inaccessible to most. */
            // ex IntentRunHost
            Host
        };

        class Guard;

        /** Constructor.
            Makes an empty object with no locked games. */
        GameArbiter();

        /** Destructor. */
        ~GameArbiter();

        /** Lock a game.
            Every lock() must eventually be followed by an unlock() with the same parameters; see Guard.
            \param gameId Game Id
            \param i Intent
            \throw std::runtime_error if game already blocked by a conflicting lock */
        void lock(int32_t gameId, Intent i);

        /** Unlock a game.
            \param gameId Game Id
            \param i Intent (same as for lock()) */
        void unlock(int32_t gameId, Intent i);

     private:
        std::set<int32_t> m_lockedGames;
    };


    /** GameArbiter guard.
        When constructed, calls GameArbiter::lock().
        When destroyed, calls GameArbiter::unlock() and thus ensures release of the lock. */
    class GameArbiter::Guard : afl::base::Uncopyable {
     public:
        /** Constructor.
            \param a GameArbiter
            \param gameId Game Id
            \param i Intent
            \throw std::runtime_error if game already blocked by a conflicting lock */
        Guard(GameArbiter& a, int32_t gameId, Intent i);

        /** Destructor. */
        ~Guard();

     private:
        GameArbiter& m_arbiter;
        const int32_t m_gameId;
        const Intent m_intent;
    };

} }

#endif
