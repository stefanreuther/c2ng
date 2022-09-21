/**
  *  \file game/sim/parallelrunner.cpp
  */

#include "game/sim/parallelrunner.hpp"
#include "afl/sys/mutexguard.hpp"

game::sim::ParallelRunner::ParallelRunner(const Setup& setup,
                                          const Configuration& opts,
                                          const game::spec::ShipList& list,
                                          const game::config::HostConfiguration& config,
                                          const game::vcr::flak::Configuration& flakConfig,
                                          afl::sys::LogListener& log,
                                          util::RandomNumberGenerator& rng,
                                          size_t numThreads)
    : Runner(setup, opts, list, config, flakConfig, log, rng),
      m_mutex(),
      m_limit(),
      m_pStopper(),
      m_threads(),
      m_startSignal(0),
      m_stopSignal(0),
      m_terminateSignal()
{
    for (size_t i = 0; i < numThreads; ++i) {
        m_threads.pushBackNew(new afl::sys::Thread("game.sim.runner", *this))->start();
    }
}

game::sim::ParallelRunner::~ParallelRunner()
{
    ParallelRunner::stop();
    for (size_t i = 0, n = m_threads.size(); i < n; ++i) {
        m_threads[i]->join();
    }
}

void
game::sim::ParallelRunner::run(Limit_t limit, util::StopSignal& stopper)
{
    // Save parameters where threads can find them
    m_limit = limit;
    m_pStopper = &stopper;

    // Start all threads
    startAll();

    // Wait for all threads to come to rest
    for (size_t i = 0, n = m_threads.size(); i < n; ++i) {
        m_stopSignal.wait();
    }

    // Clear
    m_limit = Limit_t();
    m_pStopper = 0;
}

void
game::sim::ParallelRunner::startAll()
{
    for (size_t i = 0, n = m_threads.size(); i < n; ++i) {
        m_startSignal.post();
    }
}

bool
game::sim::ParallelRunner::processRequest()
{
    // Fetch job
    std::auto_ptr<Job> j;
    {
        afl::sys::MutexGuard g(m_mutex);
        j.reset(makeJob(m_limit, *m_pStopper));
    }
    if (!j.get()) {
        return false;
    }

    // Do it
    runJob(j.get());

    // Put back
    {
        afl::sys::MutexGuard g(m_mutex);
        finishJob(j.release());
    }
    return true;
}

void
game::sim::ParallelRunner::stop()
{
    m_terminateSignal.set();
    startAll();
}

void
game::sim::ParallelRunner::run()
{
    while (1) {
        // Wait for control thread to give start signal
        m_startSignal.wait();

        // Termination check?
        if (m_terminateSignal.get()) {
            break;
        }

        // Process requests
        while (processRequest()) {
            // nix
        }

        // Signal control thread that we stop
        m_stopSignal.post();
    }
}
