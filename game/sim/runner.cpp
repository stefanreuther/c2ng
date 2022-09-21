/**
  *  \file game/sim/runner.cpp
  */

#include "game/sim/runner.hpp"
#include "game/sim/run.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/result.hpp"
#include "afl/sys/time.hpp"

/*
 *  Job (internal utility class)
 */

game::sim::Runner::Job::~Job()
{ }

inline
game::sim::Runner::Job::Job(const Setup& setup,
                            const Configuration& opts,
                            const game::spec::ShipList& list,
                            const game::config::HostConfiguration& config,
                            const game::vcr::flak::Configuration& flakConfig,
                            afl::sys::LogListener& log,
                            util::RandomNumberGenerator& rng,
                            size_t serial)
    : m_setup(setup),
      m_newState(setup),
      m_options(opts),
      m_shipList(list),
      m_config(config),
      m_flakConfiguration(flakConfig),
      m_log(log),
      m_rng(rng.getSeed() ^ uint32_t(serial)),
      m_result(),
      m_stats()
{
    m_rng();
    m_result.init(opts, int(serial));
}

inline void
game::sim::Runner::Job::run()
{
    try {
        runSimulation(m_newState, m_stats, m_result, m_options, m_shipList, m_config, m_flakConfiguration, m_rng);
    }
    catch (std::exception& e) {
        // In a correctly working system, this place is never reached.
        // This exception occurs if the VCR refuses to accept a record created by the simulator.
        // It is important to catch it anyway because if we don't, the UI will hang.
        m_log.write(afl::sys::LogListener::Error, "game.sim", "Error in VCR", e);
    }
}

inline bool
game::sim::Runner::Job::writeBack(ResultList& list) const
{
    if (m_result.battles.get() == 0 || m_result.battles->getNumBattles() == 0) {
        return false;
    } else {
        list.addResult(m_setup, m_newState, m_stats, m_result);
        return true;
    }
}

inline size_t
game::sim::Runner::Job::getSeriesLength() const
{
    return m_result.series_length;
}


/*
 *  Runner
 */

game::sim::Runner::Runner(const Setup& setup,
                          const Configuration& opts,
                          const game::spec::ShipList& list,
                          const game::config::HostConfiguration& config,
                          const game::vcr::flak::Configuration& flakConfig,
                          afl::sys::LogListener& log,
                          util::RandomNumberGenerator& rng)
    : m_setup(setup),
      m_options(opts),
      m_shipList(list),
      m_config(config),
      m_flakConfiguration(flakConfig),
      m_log(log),
      m_rng(rng),
      m_count(0),
      m_seriesLength(0),
      m_lastUpdate(0),
      m_updateInterval(500),
      m_resultList()
{ }

bool
game::sim::Runner::init()
{
    // ex WSimResultWindow::runFirstSimulation (sort-of)
    bool ok;
    if (m_count == 0) {
        Job j(m_setup, m_options, m_shipList, m_config, m_flakConfiguration, m_log, m_rng, 0);
        j.run();
        if (j.writeBack(m_resultList)) {
            m_count = 1;
            m_seriesLength = j.getSeriesLength();
            m_lastUpdate = afl::sys::Time::getTickCounter();
            ok = true;
        } else {
            ok = false;
        }
    } else {
        ok = true;
    }
    return ok;
}

const game::sim::ResultList&
game::sim::Runner::resultList() const
{
    return m_resultList;
}

void
game::sim::Runner::setUpdateInterval(uint32_t interval)
{
    m_updateInterval = interval;
}

game::sim::Runner::Limit_t
game::sim::Runner::makeSeriesLimit() const
{
    if (m_seriesLength != 0) {
        return makeFiniteLimit(m_seriesLength - (m_count % m_seriesLength));
    } else {
        return makeFiniteLimit(1);
    }
}

game::sim::Runner::Limit_t
game::sim::Runner::makeNoLimit() const
{
    return Limit_t(0);
}

game::sim::Runner::Limit_t
game::sim::Runner::makeFiniteLimit(size_t n) const
{
    return m_count + n;
}

game::sim::Runner::Job*
game::sim::Runner::makeJob(Limit_t& limit, util::StopSignal& stopper)
{
    if (!stopper.get() && (limit == 0 || m_count < limit)) {
        return new Job(m_setup, m_options, m_shipList, m_config, m_flakConfiguration, m_log, m_rng, m_count++);
    } else {
        return 0;
    }
}

void
game::sim::Runner::finishJob(Job* p)
{
    std::auto_ptr<Job> pp(p);
    pp->writeBack(m_resultList);

    uint32_t now = afl::sys::Time::getTickCounter();
    uint32_t elapsed = now - m_lastUpdate;
    if (elapsed >= m_updateInterval) {
        m_lastUpdate = now;
        sig_update.raise();
    }
}

void
game::sim::Runner::runJob(Job* p)
{
    p->run();
}
