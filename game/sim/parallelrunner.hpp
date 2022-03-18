/**
  *  \file game/sim/parallelrunner.hpp
  *  \brief game::sim::ParallelRunner
  */
#ifndef C2NG_GAME_SIM_PARALLELRUNNER_HPP
#define C2NG_GAME_SIM_PARALLELRUNNER_HPP

#include "afl/base/stoppable.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/sys/mutex.hpp"
#include "afl/sys/semaphore.hpp"
#include "afl/sys/thread.hpp"
#include "game/sim/runner.hpp"

namespace game { namespace sim {

    /** Multi-threaded simulation runner.
        Contains a configurable number of threads to run simulations.
        Threads live as long as the ParallelRunner lives.
        Each thread processes jobs.

        Worker threads work on the original versions of the setup, configuration, ship list, host configuration.
        The sig_update may therefore not modify any of those.
        The sig_update callback may come from any thread.

        Worker threads are passive when run() is not active. */
    class ParallelRunner : public Runner,
                           private afl::base::Stoppable
    {
     public:
        /** Constructor.
            \param [in]     setup   Simulation setup (see Runner)
            \param [in]     opts    Simulation options (see Runner)
            \param [in]     list    Ship list (see Runner)
            \param [in]     config  Host configuration (see Runner)
            \param [in]     flakConfig FLAK configuration (see Runner)
            \param [in,out] rng     Random number generator (see Runner)
            \param [in]     numThreads Number of threads to start*/
        ParallelRunner(const Setup& setup,
                       const Configuration& opts,
                       const game::spec::ShipList& list,
                       const game::config::HostConfiguration& config,
                       const game::vcr::flak::Configuration& flakConfig,
                       util::RandomNumberGenerator& rng,
                       size_t numThreads);

        /** Destructor.
            Stops all the threads. */
        ~ParallelRunner();

        // Runner:
        void run(Limit_t limit, util::StopSignal& stopper);

     private:
        void startAll();
        bool processRequest();

        // Stoppable:
        void run();
        void stop();

        /** Mutex protecting makeJob(), finishJob(). */
        afl::sys::Mutex m_mutex;

        /** "limit" parameter from run() for threads to see. */
        Limit_t m_limit;

        /** "stopper" parameter from run() for threads to see. */
        util::StopSignal* m_pStopper;

        /** List of threads.
            Each runs this as its Stoppable/Runnable.
            Threads are started/stopped by signaling the respective semaphore N times. */
        afl::container::PtrVector<afl::sys::Thread> m_threads;

        /** Start signal for threads.
            Set by control code (public run()) to tell threads to consider m_terminateSignal and makeJob(). */
        afl::sys::Semaphore m_startSignal;

        /** Stop signal.
            Set by threads to signal completion (makeJob() produced no more jobs). */
        afl::sys::Semaphore m_stopSignal;

        /** Termination signal.
            If a thread sees this, it terminates (call join() next). */
        util::StopSignal m_terminateSignal;
    };

} }

#endif
