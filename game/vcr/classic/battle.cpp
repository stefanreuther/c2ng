/**
  *  \file game/vcr/classic/battle.cpp
  */

#include "game/vcr/classic/battle.hpp"
#include "util/translation.hpp"
#include "game/vcr/classic/algorithm.hpp"
#include "game/vcr/classic/nullvisualizer.hpp"
#include "game/vcr/classic/hostalgorithm.hpp"
#include "game/vcr/classic/pvcralgorithm.hpp"
#include "afl/string/format.hpp"
#include "game/vcr/classic/utils.hpp"

namespace {
    int getBuildPointMass(const game::vcr::Object& obj,
                          const game::config::HostConfiguration& config,
                          const game::spec::ShipList& shipList,
                          bool isPHost)
    {
        // vcrplay.pas::PALMass, game/classicvcr.cc:getBuildPointMass
        int guessedHull = obj.getGuessedHull(shipList.hulls());
        if (obj.isPlanet()) {
            // planet
            return obj.getMass() - 100;
        } else if ((!isPHost || !config[config.PALIncludesESB](obj.getOwner())) && guessedHull != 0) {
            // ship, type known, and we have HOST or PHost where PAL does not include ESB
            if (const game::spec::Hull* hull = shipList.hulls().get(guessedHull)) {
                return hull->getMass();
            } else {
                return obj.getMass();
            }
        } else {
            // ship, type unknown, or build points include ESB
            return obj.getMass();
        }
    }
}


// Constructor.
game::vcr::classic::Battle::Battle(const Object& left,
                                   const Object& right,
                                   uint16_t seed,
                                   uint16_t signature,
                                   uint16_t planetTemperatureCode)
    : m_seed(seed),
      m_signature(signature),
      m_planetTemperatureCode(planetTemperatureCode),
      m_result(),
      m_type(Unknown),
      m_capabilities(0)
{
    // ex GClassicVcr::GClassicVcr
    // ex GClassicVcrEntry::GClassicVcrEntry
    m_before[0] = left;
    m_before[1] = right;
}

// Destructor.
game::vcr::classic::Battle::~Battle()
{ }

size_t
game::vcr::classic::Battle::getNumObjects() const
{
    // ex GClassicVcrEntry::getNumObjects
    return 2;
}

game::vcr::Object*
game::vcr::classic::Battle::getObject(size_t slot, bool after)
{
    // ex GClassicVcrEntry::getObject
    if (slot == 0 || slot == 1) {
        if (after) {
            return &m_after[slot];
        } else {
            return &m_before[slot];
        }
    } else {
        return 0;
    }
}

int
game::vcr::classic::Battle::getOutcome(const game::config::HostConfiguration& config,
                                       const game::spec::ShipList& shipList,
                                       size_t slot)
{
    // ex GClassicVcrEntry::getOutcome
    // Make sure result is known
    if (m_result.empty()) {
        prepareResult(config, shipList, NeedQuickOutcome);
    }

    // Process it
    if (m_result.contains(Invalid)) {
        // Error
        return 0;
    } else {
        if (slot == 0) {
            if (m_result.contains(LeftDestroyed)) {
                return -1;
            } else if (m_result.contains(LeftCaptured)) {
                return m_before[1].getOwner();
            } else {
                return 0;
            }
        } else {
            if (m_result.contains(RightDestroyed)) {
                return -1;
            } else if (m_result.contains(RightCaptured)) {
                return m_before[0].getOwner();
            } else {
                return 0;
            }
        }
    }
}

game::vcr::Battle::Playability
game::vcr::classic::Battle::getPlayability(const game::config::HostConfiguration& config,
                                           const game::spec::ShipList& shipList)
{
    // ex GClassicVcrEntry::getPlayability
    if (m_result.empty()) {
        // Result not known, perform check
        NullVisualizer vis;
        std::auto_ptr<Algorithm> algo(createAlgorithm(vis, config, shipList));
        if (!algo.get()) {
            m_result += Invalid;
        } else {
            Object left(m_before[0]);
            Object right(m_before[1]);
            uint16_t seed(m_seed);
            if (!algo->setCapabilities(m_capabilities) || algo->checkBattle(left, right, seed)) {
                m_result += Invalid;
            }
        }
    }
    return m_result.contains(Invalid) ? IsDamaged : IsPlayable;
}

void
game::vcr::classic::Battle::prepareResult(const game::config::HostConfiguration& config,
                                          const game::spec::ShipList& shipList,
                                          int /*resultLevel*/)
{
    // ex GClassicVcrEntry::prepareResult
    if (m_result.empty()) {
        // Result not known, so compute it
        NullVisualizer vis;
        std::auto_ptr<Algorithm> algo(createAlgorithm(vis, config, shipList));
        if (!algo.get()) {
            // Cannot be played
            m_result += Invalid;
        } else {
            // Try to play
            Object left(m_before[0]);
            Object right(m_before[1]);
            uint16_t seed(m_seed);
            if (!algo->setCapabilities(m_capabilities) || algo->checkBattle(left, right, seed)) {
                // Cannot be played
                m_result += Invalid;
            } else {
                // Can be played
                algo->playBattle(left, right, seed);
                algo->doneBattle(m_after[0], m_after[1]);
                m_result = algo->getResult();
            }
        }
    }
}

String_t
game::vcr::classic::Battle::getAlgorithmName(afl::string::Translator& tx) const
{
    // ex GClassicVcrEntry::getAlgorithmName
    static const char*const NAMES[] = {
        // xref enum Type
        N_("Unknown"),
        N_("Host"),
        N_("Unknown PHost"),
        N_("PHost 2"),
        N_("PHost 3"),
        N_("PHost 4"),
        N_("NuHost"),
    };
    return tx.translateString(NAMES[m_type]);
}

bool
game::vcr::classic::Battle::isESBActive(const game::config::HostConfiguration& config) const
{
    // ex GClassicVcrEntry::isESBActive
    // Simple cases
    if (!config[config.AllowEngineShieldBonus]()) {
        return false;
    }
    if (config[config.AllowESBonusAgainstPlanets]()) {
        return true;
    }

    // ESB is active, but not against planets. So check for planet.
    if (m_before[1].isPlanet()) {
        return false;
    }
    return true;
}

/*
 *  Additional methods
 */

// Set battle type.
void
game::vcr::classic::Battle::setType(Type type, uint16_t capabilities)
{
    // ex GClassicVcrEntry::setType
    m_type = type;
    m_capabilities = capabilities;

    // invalidate result
    m_result = BattleResult_t();
}

// Get battle type.
game::vcr::classic::Type
game::vcr::classic::Battle::getType() const
{
    // ex GClassicVcrEntry::getType
    return m_type;
}

// Get battle type.
uint16_t
game::vcr::classic::Battle::getCapabilities() const
{
    // ex GClassicVcrEntry::getCapabilities
    return m_capabilities;
}

// Get battle signature.
uint16_t
game::vcr::classic::Battle::getSignature() const
{
    // ex GClassicVcr::getSignature
    return m_signature;
}

// Get random number seed.
uint16_t
game::vcr::classic::Battle::getSeed() const
{
    // ex GClassicVcr::getSeed
    return m_seed;
}

// Format current status as string.
String_t
game::vcr::classic::Battle::formatResult(int player, const String_t& annotation, afl::string::Translator& tx) const
{
    // ex VcrPlayer::getResultString
    return formatBattleResult(m_result,
                              m_before[0].getName(),
                              (m_before[0].getOwner() == player ? TeamSettings::ThisPlayer : TeamSettings::EnemyPlayer),
                              m_before[1].getName(),
                              (m_before[1].getOwner() == player ? TeamSettings::ThisPlayer : TeamSettings::EnemyPlayer),
                              annotation,
                              tx);
}

game::vcr::classic::BattleResult_t
game::vcr::classic::Battle::getResult() const
{
    return m_result;
}


// /** Store result. Fetches the result from the specified
//     VCR Player. The player must have called playVcr(), but
//     not yet doneVcr(). */
// void
// GClassicVcrEntry::setResultFromPlayer(VcrPlayer& player)
// {
//     player.doneVcr(result);
//     status = player.getStatusWord();
// }

// Create a player algorithm that can play this battle.
game::vcr::classic::Algorithm*
game::vcr::classic::Battle::createAlgorithm(Visualizer& vis,
                                            const game::config::HostConfiguration& config,
                                            const game::spec::ShipList& shipList)
{
    // ex GClassicVcrEntry::createPlayer
    return createAlgorithmForType(m_type, vis, config, shipList);
}

// Create a player algorithm for a given algorithm name
game::vcr::classic::Algorithm*
game::vcr::classic::Battle::createAlgorithmForType(Type type,
                                                   Visualizer& vis,
                                                   const game::config::HostConfiguration& config,
                                                   const game::spec::ShipList& shipList)
{
    // ex GClassicVcrEntry::createPlayerForType
    // NOTE: caller must call setCapabilities!
    const game::spec::BeamVector_t& beams = shipList.beams();
    const game::spec::TorpedoVector_t& launchers = shipList.launchers();
    switch (type) {
     case Host:
        return new HostAlgorithm(false, vis, config, beams, launchers);
     case NuHost:
        return new HostAlgorithm(true, vis, config, beams, launchers);
     case PHost4:
        return new PVCRAlgorithm(true, vis, config, beams, launchers);
     case PHost3:
        return new PVCRAlgorithm(true, vis, config, beams, launchers);
     case PHost2:
        return new PVCRAlgorithm(false, vis, config, beams, launchers);
     case Unknown:
     case UnknownPHost:
        break;

    }
    return 0;
}

// Compute scores.
void
game::vcr::classic::Battle::computeScores(Score& score,
                                          const Side side,
                                          const game::config::HostConfiguration& config,
                                          const game::spec::ShipList& shipList) const
{
    // ex GClassicVcrEntry::computeScores

    // true iff we destroyed the other one
    const bool isVictor = (side != LeftSide ? m_result.contains(LeftDestroyed) : m_result.contains(RightDestroyed));

    // true iff we were not destroyed nor captured
    const bool didSurvive = (side != LeftSide
                             ? (!m_result.contains(RightDestroyed) && !m_result.contains(RightCaptured))
                             : (!m_result.contains(LeftDestroyed)  && !m_result.contains(LeftCaptured)));

    const int opp = !side;
    const int myRace = m_before[side].getOwner();

    if (m_type == PHost2 || m_type == PHost3 || m_type == PHost4 || m_type == UnknownPHost) {
        const int32_t damageDone = m_after[opp].getDamage() - m_before[opp].getDamage();
        const int32_t theirMass = std::max(getBuildPointMass(m_before[opp], config, shipList, true), 1);
        const int32_t myMass = std::max(getBuildPointMass(m_before[side], config, shipList, true), 1);

        // Build points. PHost gives different points for aggressor and opponent
        int32_t aggMP = damageDone * theirMass * config[config.PALAggressorPointsPer10KT](myRace);
        int32_t oppMP = damageDone * theirMass * config[config.PALOpponentPointsPer10KT](myRace);
        if (isVictor) {
            // We won
            aggMP += theirMass * 100 * config[config.PALAggressorKillPointsPer10KT](myRace);
            oppMP += theirMass * 100 * config[config.PALOpponentKillPointsPer10KT](myRace);
        } else if (!m_before[opp].isPlanet() && m_result.contains(side != LeftSide ? LeftCaptured : RightCaptured)) {
            // We captured them
            int32_t crewKilled = m_before[opp].getCrew() - m_after[opp].getCrew();
            int32_t pts = crewKilled * config[config.PALShipCapturePer10Crew](myRace) * 100;
            aggMP += pts;
            oppMP += pts;
        }

        // Swap, so that aggMP is the bigger value
        if (oppMP > aggMP) {
            std::swap(aggMP, oppMP);
        }

        if (m_before[opp].isPlanet()) {
            // Scale planet points. Scale big*big, small*small
            int scale1 = config[config.PALCombatBaseScaling](myRace);
            int scale2 = config[config.PALCombatPlanetScaling](myRace);
            if (scale1 > scale2) {
                std::swap(scale1, scale2);
            }
            oppMP = (oppMP * scale1) / 100;
            aggMP = (aggMP * scale2) / 100;
        }
        score.addBuildMillipoints(oppMP, aggMP);

        // Experience
        if (didSurvive && config[config.NumExperienceLevels]() > 0) {
            // FIXME: EPCombatBoostLevel / EPCombatBoostRate
            score.addExperience((damageDone * theirMass * config[config.EPCombatDamageScaling]()) / (100 * myMass));
            if (isVictor) {
                score.addExperience(theirMass * config[config.EPCombatKillScaling]() / myMass);
            }
        }
    } else {
        // Build points only for destruction of other side, and only for ship/ship fight.
        if (isVictor && !m_before[RightSide].isPlanet()) {
            const int32_t bmp = 1000 * ((getBuildPointMass(m_before[opp], config, shipList, false) / 100) + 1);
            score.addBuildMillipoints(bmp, bmp);
        }

        // Experience.
        // Actually, HOST gives +20 for destroying a ship, +40 for capturing one,
        // but since players don't see experience, pretend there is none.
    }

    // Tons are common for all hosts
    if (isVictor) {
        score.addTonsDestroyed(m_before[opp].getMass());
    }
}


// FIXME: remove, no longer needed
// inline void
// GClassicVcr::setSeed(uint16_t seed)
// {
//     this->seed = seed;
// }

// FIXME: remove, no longer needed
// inline void
// GClassicVcr::setSignature(uint16_t signature)
// {
//     this->signature = signature;
// }
