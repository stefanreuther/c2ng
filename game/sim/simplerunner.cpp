/**
  *  \file game/sim/simplerunner.cpp
  *  \brief Class game::sim::SimpleRunner
  */

#include <memory>
#include "game/sim/simplerunner.hpp"



game::sim::SimpleRunner::SimpleRunner(const Setup& setup,
                                      const Configuration& opts,
                                      const game::spec::ShipList& list,
                                      const game::config::HostConfiguration& config,
                                      const game::vcr::flak::Configuration& flakConfig,
                                      afl::sys::LogListener& log,
                                      util::RandomNumberGenerator& rng)
    : Runner(setup, opts, list, config, flakConfig, log, rng)
{ }

void
game::sim::SimpleRunner::run(Limit_t limit, util::StopSignal& stopper)
{
    // ex WSimResultWindow::runSimulation (sort-of)
    while (1) {
        std::auto_ptr<Job> p(makeJob(limit, stopper));
        if (p.get() == 0) {
            break;
        }
        runJob(p.get());
        finishJob(p.release());
    }
}
