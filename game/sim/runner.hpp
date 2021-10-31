/**
  *  \file game/sim/runner.hpp
  *  \brief Class game::sim::Runner
  */
#ifndef C2NG_GAME_SIM_RUNNER_HPP
#define C2NG_GAME_SIM_RUNNER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/signal.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/sim/resultlist.hpp"
#include "game/sim/setup.hpp"
#include "game/spec/shiplist.hpp"
#include "game/vcr/flak/configuration.hpp"
#include "util/randomnumbergenerator.hpp"
#include "util/stopsignal.hpp"

namespace game { namespace sim {

    class Configuration;

    /** Simulation runner base class.
        A simulation runner runs a simulation multiple times (runSimulation)
        and collects results in a ResultList.

        This class provides the framework and interface;
        derived classes map the simulation execution to threads.

        Usage:
        - create Runner
        - optional: hook sig_update, configure setUpdateInterval()
        - call init() to run the first simulation
        - call run() (implemented by derived class) to run more simulations */
    class Runner : public afl::base::Deletable {
     public:
        /** Opaque class to represent a simulation job. */
        struct Job;

        /** Opaque data type to represent a simulation count limit. */
        typedef size_t Limit_t;

        /** Constructor.
            \param [in]     setup   Simulation setup
            \param [in]     opts    Simulation options
            \param [in]     list    Ship list
            \param [in]     config  Host configuration
            \param [in]     flakConfig FLAK configuration
            \param [in/out] rng     Random number generator */
        Runner(const Setup& setup,
               const Configuration& opts,
               const game::spec::ShipList& list,
               const game::config::HostConfiguration& config,
               const game::vcr::flak::Configuration& flakConfig,
               util::RandomNumberGenerator& rng);

        /** Initialize.
            This computes the first simulation. */
        bool init();

        /** Run more simulations.
            Computes more simulations until the specified count limit has been reached,
            or the StopSignal signals stop.

            \param [in]     limit    Count limit. Use makeSeriesLimit(), makeNoLimit(), makeFiniteLimit() to create.
            \param [in/out] stopper  Can be signaled by another thread to stop early

            Implementations must repeatedly
            - call makeJob() with the given parameters
            - if it returns non-null, call runJob(), then finishJob().

            If the implementation uses multiple threads,
            it must make sure that makeJob() and finishJob() are run under mutex protection;
            runJob() can run in parallel.

            \todo Do we need more restrictions for makeJob(), finishJob()?
            Requiring them to run in the thread that called run() requires implementors to do some kind of queue,
            but would allow callbacks generated from finishJob() to be generated from a fixed thread. */
        virtual void run(Limit_t limit, util::StopSignal& stopper) = 0;

        /** Access result list.
            \return handle to result list */
        const ResultList& resultList() const;

        /** Set update interval.
            You will receive sig_update updates about every so many milliseconds.
            \param interval Interval */
        void setUpdateInterval(uint32_t interval);

        /** Make limit: series.
            If makeSeriesLimit() is passed as limit to run(),
            the current series will be computed until completion or,
            if it is already complete, another series will be run.

            \return limit value. Value is only meaningful until next run() invocation. */
        Limit_t makeSeriesLimit() const;

        /** Make limit: no limit.
            If makeNoLimit() is passed as limit to run(),
            run() will run until stopped by StopSignal,
            but will not stop on its own.

            \return limit value. Value is only meaningful until next run() invocation. */
        Limit_t makeNoLimit() const;

        /** Make limit: finite count.
            If makeFiniteLimit(n) is passed as limit to run(),
            exactly n more simulations will be run.

            \return limit value. Value is only meaningful until next run() invocation. */
        Limit_t makeFiniteLimit(size_t n) const;

        /** Signal: update.
            Called whenever new simulations have been produced and the configured update interval has elapsed */
        afl::base::Signal<void()> sig_update;

     protected:
        /** Create new job.
            Call from your run(), see there.
            \param limit   Limit
            \param stopper Stopper
            \return newly-allocated job; null to stop simulating. */
        Job* makeJob(Limit_t& limit, util::StopSignal& stopper);

        /** Finish a job.
            Call from your run(), see there.
            \param p Job created by makeJob(), you must have called runJob(). */
        void finishJob(Job* p);

        /** Run a job.
            Call from your run(), see there.
            \param p Job created by makeJob(). */
        static void runJob(Job* p);

     private:
        const Setup& m_setup;
        const Configuration& m_options;
        const game::spec::ShipList& m_shipList;
        const game::config::HostConfiguration& m_config;
        const game::vcr::flak::Configuration& m_flakConfiguration;
        util::RandomNumberGenerator& m_rng;

        /** Number of started simulations (=number of Job objects created).
            We need a serial number for each sim to do seed control.

            ResultList contains a number of completed simulations,
            which is used by PCC2 for this purpose, but cannot be used
            if an unknown number of jobs can be in flight. */
        size_t m_count;

        /** Series length.
            Instead of storing the last Result object,
            we just stash aways the initial battle's value here. */
        size_t m_seriesLength;

        /** Time of last signalisation. */
        uint32_t m_lastUpdate;

        /** Signalisation interval. */
        uint32_t m_updateInterval;

        /** Result accumulator. */
        ResultList m_resultList;
    };

} }

/* Internal helper class.
   Its definition needs to be visible so users can make std::auto_ptr's pointing to it.
   Therefore, it also needs a public destructor.

   However, its members are intended to be used by Runner only,
   and are therefore private and inline. */
class game::sim::Runner::Job {
 public:
    ~Job();

 private:
    friend class Runner;

    inline Job(const Setup& setup, const Configuration& opts, const game::spec::ShipList& list, const game::config::HostConfiguration& config,
               const game::vcr::flak::Configuration& flakConfig, util::RandomNumberGenerator& rng, size_t serial);
    inline void run();
    inline bool writeBack(ResultList& list) const;
    inline size_t getSeriesLength() const;

    const Setup& m_setup;
    Setup m_newState;
    const Configuration& m_options;
    const game::spec::ShipList& m_shipList;
    const game::config::HostConfiguration& m_config;
    const game::vcr::flak::Configuration& m_flakConfiguration;
    util::RandomNumberGenerator m_rng;
    Result m_result;
    std::vector<game::vcr::Statistic> m_stats;
};


#endif
