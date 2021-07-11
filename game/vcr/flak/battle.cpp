/**
  *  \file game/vcr/flak/battle.cpp
  *  \brief Class game::vcr::flak::Battle
  */

#include "game/vcr/flak/battle.hpp"
#include "game/vcr/flak/algorithm.hpp"
#include "game/vcr/flak/gameenvironment.hpp"
#include "game/vcr/flak/nullvisualizer.hpp"
#include "game/vcr/score.hpp"

namespace {
    /*
     *  FLAK Score Heuristics (Battle::computeScores).
     *
     *  FLAK does not give individual scores.
     *  Instead, it computes per-player totals.
     *  Each unit receives the same experience from that; and for build points/tons, there's only on total per player anyway.
     *  Since we need per-player totals, we split the build points/tons among the surviving units.
     *
     *  As an additional obstacle, we cannot determine from a fight's client-side view how points are distributed.
     *  If two players destroy a third's ship, points are given to whomever placed the last shot.
     *  In that case, we therefore return a range including 0.
     */
    struct Totals {
        int numAliveUnits;
        int numAliveUnitsBefore;
        int32_t totalExpMass;
        int32_t totalExpMin;
        int32_t totalExpMax;
        int32_t tonsDestroyedMin;
        int32_t tonsDestroyedMax;
        int32_t buildMillipointsMin;
        int32_t buildMillipointsMax;
        Totals()
            : numAliveUnits(), numAliveUnitsBefore(),
              totalExpMass(), totalExpMin(), totalExpMax(),
              tonsDestroyedMin(), tonsDestroyedMax(),
              buildMillipointsMin(), buildMillipointsMax()
            { }
    };

    int32_t splitPoints(int32_t points, int32_t units, int32_t unitsBefore)
    {
        int32_t result = points / units;
        int32_t rem    = points % units;
        if (rem > unitsBefore) {
            ++result;
        }
        return result;
    }

    void addPointsForKill(Totals& t, const game::vcr::Object& obj, int meOwner, bool sureKill, const game::config::HostConfiguration& config) {
        if (obj.isPlanet()) {
            int32_t mass = obj.getMass() - 100;
            int32_t baseScale   = config[game::config::HostConfiguration::PALCombatBaseScaling](meOwner);
            int32_t planetScale = config[game::config::HostConfiguration::PALCombatPlanetScaling](meOwner);
            int32_t points      = config[game::config::HostConfiguration::PALAggressorPointsPer10KT](meOwner) + config[game::config::HostConfiguration::PALAggressorPointsPer10KT](meOwner);
            int32_t minScale = std::min(baseScale, planetScale);
            int32_t maxScale = std::max(baseScale, planetScale);
            if (sureKill) {
                t.buildMillipointsMin += mass * minScale * points;
            }
            t.buildMillipointsMax += mass * maxScale * points;

            int32_t exp = mass * (config[game::config::HostConfiguration::EPCombatDamageScaling]() + config[game::config::HostConfiguration::EPCombatKillScaling]());
            if (sureKill) {
                t.totalExpMin += exp;
            }
            t.totalExpMax += exp;
        } else {
            int32_t damageLimit = config.getPlayerRaceNumber(obj.getOwner()) == 2 ? 151 : 100;
            int32_t damageDone = damageLimit - obj.getDamage();
            int32_t mass = obj.getMass();

            int32_t points = mass * config[game::config::HostConfiguration::PALAggressorPointsPer10KT](meOwner) * damageDone
                + mass * config[game::config::HostConfiguration::PALAggressorKillPointsPer10KT](meOwner) * 100;
            if (sureKill) {
                t.buildMillipointsMin += points;
            }
            t.buildMillipointsMax += points;

            int32_t exp = mass * config[game::config::HostConfiguration::EPCombatDamageScaling]() * damageDone / 100
                + mass * config[game::config::HostConfiguration::EPCombatKillScaling]();
            if (sureKill) {
                t.totalExpMin += exp;
            }
            t.totalExpMax += exp;

            if (sureKill) {
                t.tonsDestroyedMin += mass;
            }
            t.tonsDestroyedMax += mass;
        }
    }

    void addPointsForCapture(Totals& t,
                             const game::vcr::Object& before,
                             const game::vcr::Object& after,
                             int meOwner, const game::config::HostConfiguration& config) {
        if (before.isPlanet()) {
            // For planet, kill and capture is the same
            addPointsForKill(t, before, meOwner, true, config);
        } else {
            // Points for capturing
            int damageDone = after.getDamage() - before.getDamage();
            int mass = before.getMass();
            int32_t pointsForCapture = mass * config[game::config::HostConfiguration::PALAggressorPointsPer10KT](meOwner) * damageDone
                + 100 * before.getCrew() * config[game::config::HostConfiguration::PALShipCapturePer10Crew](meOwner);

            t.buildMillipointsMin += pointsForCapture;
            t.buildMillipointsMax += pointsForCapture;

            // Experience
            int32_t exp = mass * config[game::config::HostConfiguration::EPCombatDamageScaling]() * damageDone / 100;
            t.totalExpMin += exp;
            t.totalExpMax += exp;
        }
    }
}

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

size_t
game::vcr::flak::Battle::getNumGroups() const
{
    return m_setup->getNumFleets();
}

game::vcr::GroupInfo
game::vcr::flak::Battle::getGroupInfo(size_t groupNr, const game::config::HostConfiguration& /*config*/) const
{
    if (groupNr < getNumGroups()) {
        const Setup::Fleet& f = m_setup->getFleetByIndex(groupNr);
        return GroupInfo(f.firstShipIndex, f.numShips, f.x, f.y, f.player, f.speed);
    } else {
        return GroupInfo();
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
            Algorithm algo(*m_setup, env);
            algo.init(env, vis);
            while (algo.playCycle(env, vis))
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

afl::base::Optional<int32_t>
game::vcr::flak::Battle::getAuxiliaryInformation(AuxInfo info) const
{
    switch (info) {
     case aiSeed:     return static_cast<int32_t>(m_setup->getSeed());
     case aiMagic:    break;  // only for classic
     case aiType:     break;  // only for classic
     case aiFlags:    break;  // only for classic
     case aiAmbient:  return m_setup->getAmbientFlags();
    }
    return afl::base::Nothing;
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

bool
game::vcr::flak::Battle::computeScores(Score& score, size_t slot,
                                       const game::config::HostConfiguration& config,
                                       const game::spec::ShipList& shipList) const
{
    // Get initial objects. Must exist and survive.
    const Object* me = getObject(slot, false);
    if (me == 0 || me->getEndingStatus() != 0) {
        return false;
    }
    const int meOwner = me->getOwner();

    // Determine possible victors.
    // Points for killing are given to whoever placed the last shot (with host-side random as fallback),
    // which is not contained in the battle recording.
    // We only know it for sure if there is only one possibility.
    PlayerSet_t offensivePlayers;
    for (size_t i = 0, n = getNumObjects(); i < n; ++i) {
        if (const Object* p = getObject(i, false)) {
            if (!p->isFreighter()) {
                offensivePlayers += p->getOwner();
            }
        }
    }

    // Compute totals
    Totals t;
    for (size_t i = 0, n = getNumObjects(); i < n; ++i) {
        const Object* before = getObject(i, false);
        const Object* after = getObject(i, true);
        if (before != 0 && after != 0) {
            if (before->getOwner() == meOwner) {
                // A unit that started as one of ours.
                // Count mass for experience.
                t.totalExpMass += before->getBuildPointMass(config, shipList, true);

                if (before->getEndingStatus() == 0) {
                    // Unit survived: count number of survivors to distribute points among
                    ++t.numAliveUnits;
                    if (i < slot) {
                        ++t.numAliveUnitsBefore;
                    }
                } else if (before->getEndingStatus() == meOwner) {
                    // Capture back
                    addPointsForCapture(t, *before, *after, meOwner, config);
                } else {
                    // Our unit got captured or died: no points for us
                }
            } else {
                // Enemy unit
                if (before->getEndingStatus() < 0) {
                    // Someone (maybe we?) destroyed it.
                    bool sureKill = (offensivePlayers - meOwner - before->getOwner()).empty();
                    addPointsForKill(t, *before, meOwner, sureKill, config);
                } else if (before->getEndingStatus() == meOwner) {
                    // We captured it
                    addPointsForCapture(t, *before, *after, meOwner, config);
                } else {
                    // Survived, or someone else captured it.
                }
            }
        }
    }

    // Distribute points
    if (t.numAliveUnits != 0 && t.totalExpMass != 0) {
        // Every survivor gets the same amount of experience.
        score.addExperience(Score::Range_t(t.totalExpMin / t.totalExpMass,
                                           t.totalExpMax / t.totalExpMass));

        // Player gets some total tons/PBP amount. Split among all units, so that the sum is correct.
        score.addBuildMillipoints(Score::Range_t(splitPoints(t.buildMillipointsMin, t.numAliveUnits, t.numAliveUnitsBefore),
                                                 splitPoints(t.buildMillipointsMax, t.numAliveUnits, t.numAliveUnitsBefore)));
        score.addTonsDestroyed(Score::Range_t(splitPoints(t.tonsDestroyedMin, t.numAliveUnits, t.numAliveUnitsBefore),
                                              splitPoints(t.tonsDestroyedMax, t.numAliveUnits, t.numAliveUnitsBefore)));
        return true;
    } else {
        return false;
    }
}
