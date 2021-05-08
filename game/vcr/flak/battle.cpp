/**
  *  \file game/vcr/flak/battle.cpp
  *  \brief Class game::vcr::flak::Battle
  */

#include "game/vcr/flak/battle.hpp"
#include "game/vcr/flak/algorithm.hpp"
#include "game/vcr/flak/gameenvironment.hpp"
#include "game/vcr/flak/nullvisualizer.hpp"

game::vcr::flak::Battle::Battle(std::auto_ptr<Setup> setup)
    : m_setup(setup),
      m_after(),
      m_haveAfter(false)
{ }

size_t
game::vcr::flak::Battle::getNumObjects() const
{
    // ex FlakVcrEntry::getNumObjects
    return m_setup->getNumShips();
}

const game::vcr::flak::Object*
game::vcr::flak::Battle::getObject(size_t slot, bool after) const
{
    // ex FlakVcrEntry::getObject
    if (after) {
        // After
        if (slot >= m_after.size()) {
            return 0;
        } else {
            return m_after[slot];
        }
    } else {
        // Before
        if (slot >= m_setup->getNumShips()) {
            return 0;
        } else {
            return &m_setup->getShipByIndex(slot);
        }
    }
}

int
game::vcr::flak::Battle::getOutcome(const game::config::HostConfiguration& /*config*/,
                                    const game::spec::ShipList& /*shipList*/,
                                    size_t slot)
{
    // ex FlakVcrEntry::getOutcome
    if (slot >= m_setup->getNumShips()) {
        return 0;
    } else {
        return m_setup->getShipByIndex(slot).getEndingStatus();
    }
}

game::vcr::Battle::Playability
game::vcr::flak::Battle::getPlayability(const game::config::HostConfiguration& /*config*/,
                                        const game::spec::ShipList& /*shipList*/)
{
    // ex FlakVcrEntry::getPlayability
    return IsPlayable;
}

void
game::vcr::flak::Battle::prepareResult(const game::config::HostConfiguration& config,
                                       const game::spec::ShipList& shipList,
                                       int resultLevel)
{
    // ex FlakVcrEntry::prepareResult
    if ((resultLevel & ~NeedQuickOutcome) != 0 && !m_haveAfter) {
        // Play the fight
        if (m_setup->getNumFleets() != 0) {
            NullVisualizer vis;
            GameEnvironment env(config, shipList.beams(), shipList.launchers());
            Algorithm algo(vis, *m_setup, env);
            algo.init(env);
            while (algo.playCycle(env))
                ;

            // Build the result
            for (size_t i = 0, n = m_setup->getNumShips(); i < n; ++i) {
                Object& obj = *m_after.pushBackNew(new Object(m_setup->getShipByIndex(i)));
                algo.copyResult(i, obj);
            }
        }
        m_haveAfter = true;        
    }
}

String_t
game::vcr::flak::Battle::getAlgorithmName(afl::string::Translator& tx) const
{
    // ex FlakVcrEntry::getAlgorithmName
    return tx("FLAK");
}

bool
game::vcr::flak::Battle::isESBActive(const game::config::HostConfiguration& config) const
{
    // ex FlakVcrEntry::isESBActive
    return config[game::config::HostConfiguration::AllowEngineShieldBonus]();
}

bool
game::vcr::flak::Battle::getPosition(game::map::Point& result) const
{
    return m_setup->getPosition(result);
}

String_t
game::vcr::flak::Battle::getResultSummary(int viewpointPlayer,
                                          const game::config::HostConfiguration& config, const game::spec::ShipList& shipList,
                                          util::NumberFormatter fmt, afl::string::Translator& tx) const
{
    // FIXME
    (void) viewpointPlayer;
    (void) config;
    (void) shipList;
    (void) fmt;
    (void) tx;
    return String_t();
}
