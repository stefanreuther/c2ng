/**
  *  \file game/sim/run.hpp
  *  \brief Simulator Main Entry Point
  */
#ifndef C2NG_GAME_SIM_RUN_HPP
#define C2NG_GAME_SIM_RUN_HPP

#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"
#include "game/vcr/flak/configuration.hpp"
#include "game/vcr/statistic.hpp"
#include "util/randomnumbergenerator.hpp"

namespace game { namespace sim {

    class Setup;
    class Configuration;
    class Result;

    /** Run one simulation.

        Limitation: Statistic::getMinFightersAboard() will not be initialized for planets that do not fight.
        
        \param [in/out]  setup     Simulation state. Will be updated to contain the simulation results.
        \param [out]     stats     Receives out-of-band statistics not covered by state.
        \param [in/out]  result    Result descriptor. Caller must initialize; will be updated with new battle weights.
        \param [in]      opts      Simulator options
        \param [in]      list      Ship list (requires hulls, beams, engines, torpedo launchers, friendly codes, hull functions)
        \param [in]      config    Host configuration
        \param [in]      flakConfig FLAK configuration
        \param [in/out]  rng       Random number generator; used only of \c opts does not configure a deterministic simulation */
    void runSimulation(Setup& setup,
                       std::vector<game::vcr::Statistic>& stats,
                       Result& result,
                       const Configuration& opts,
                       const game::spec::ShipList& list,
                       const game::config::HostConfiguration& config,
                       const game::vcr::flak::Configuration& flakConfig,
                       util::RandomNumberGenerator& rng);

    /** Prepare for simulation.
        Call once before calling runSimulation() possibly multiple times.
        This will process random friendly codes for hasRandomizeFCodesOnEveryFight()=off. 
        
        \param [in/out]  setup     Simulation state. Will be updated to contain the simulation results.
        \param [in]      opts      Simulator options
        \param [in/out]  rng       Random number generator; used only of \c opts does not configure a deterministic simulation */
    void prepareSimulation(Setup& setup, const Configuration& opts, util::RandomNumberGenerator& rng);

} }

#endif
