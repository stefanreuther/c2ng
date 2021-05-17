/**
  *  \file game/sim/simplerunner.hpp
  *  \brief Class game::sim::SimpleRunner
  */
#ifndef C2NG_GAME_SIM_SIMPLERUNNER_HPP
#define C2NG_GAME_SIM_SIMPLERUNNER_HPP

#include "game/sim/runner.hpp"

namespace game { namespace sim {

    /** Simple single-threaded simulation runner.
        Runs entirely in the invoking thread. */
    class SimpleRunner : public Runner {
     public:
        SimpleRunner(const Setup& setup,
                     const Configuration& opts,
                     const game::spec::ShipList& list,
                     const game::config::HostConfiguration& config,
                     const game::vcr::flak::Configuration& flakConfig,
                     util::RandomNumberGenerator& rng);

        void run(Limit_t limit, util::StopSignal& stopper);
    };

} }

#endif
