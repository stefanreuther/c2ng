/**
  *  \file game/vcr/classic/pvcralgorithm.cpp
  *  \brief Class game::vcr::classic::PVCRAlgorithm
  */

#include <algorithm>
#include "game/vcr/classic/pvcralgorithm.hpp"
#include "game/v3/structures.hpp"
#include "game/vcr/classic/statustoken.hpp"
#include "game/vcr/classic/visualizer.hpp"
#include "util/math.hpp"

/*
  This seems to be pretty optimisation-resistant.

  All tests run on a vanilla Pentium 200. Each test is the mean of at
  least two runs.

  + original:                   AC: 15.74,  Non-AC: 3.87
  + inlining:                   AC: 15.27,  Non-AC: 3.72
  + PVCR_INTEGER:               AC: 15.25,  Non-AC: 3.74(!)
  + f_getDamage():              AC: 16.11(!) [not used]
  + caching config:             AC: 13.58,  Non-AC: 3.59
  + fighterMove:                AC: 11.91,  Non-AC: 3.41
  + beamFindNearestFighter:     AC:  6.70,  Non-AC: 2.04
  + template<int side>:         AC:  5.36,  Non-AC: 1.64  (currently not used)

  For comparison, PDK 4.2:      AC: 9.50,   Non-AC: 2.44
  PDK 4.3 (beamFindNearest.)    AC: 5.53,   Non-AC: 1.72

  The one reason why the PDK is faster is probably that the PDK
  contains for loops (for(side=0; side<2; ++side)), where this code
  contains two function calls for each phase. gcc can optimise that
  better. The speedup gained by templatizing seems to support that
  supposition.

  More optimisation: 100 test runs on Athlon XP 2800+ (2 GHz):
  (testvcr tests/pvcr/vcr5.dat 100)

  + start with:                 AC: 4.72,   Non-AC: 1.52
  + ShMovSpeed=0 for planets:   AC: 4.86,   Non-AC: 1.49  (not used)
  + fast forward:               AC: 4.82,   Non-AC: 1.45
  + randomRange100              AC: 4.53,   Non-AC: 1.30
  + uint32 seed                 AC: 4.33,   Non-AC: 1.27
  + fighterIntercept opt.: no obervable speedup in this test
    case, but 5% faster (4.4 -> 4.2) for carrier/carrier fight
  + PVCR_PREPARED_RNG           AC: 3.78,   Non-AC: 1.13
  + 20120122                    AC: 3.65,   Non-AC: 1.09
  + randomRange100LT, RunningStatus int16 for some values:
                                AC: 3.41,   Non-AC: 1.03
  + store seed<<16              AC: 3.19,   Non-AC: 0.97
  + Status contains VCR         AC: 2.64,   Non-AC: 0.83
  + side->Status&               AC: 2.57,   Non-AC: 0.82

  Disabling VCR_STATIC:
  + 20120616                    AC: 2.86,   Non-AC: 0.87
  + 20120707                    AC: 2.93,   Non-AC: 0.90
  + templatizing hit            AC: 2.81,   Non-AC: 0.87

  Note that FF applies only in 2 of 15 battles of the test case; for
  those where it applies it yields a huge speedup, so I put it in.

  Some things which do not get anything:
  + PRNG skip tables for Fast Forward: precompute how N iterations
    of randomRange(100) modify the seed for a subset of M of all
    seeds; if current seed is one of those and we have still N or
    more to go, make big jump instead of small step. With M*N=65536,
    this should get a theoretical speedup of 2 for the FF operation,
    but has no measurable overall effect. Tested with N=128.

  New tests (i5 3700 MHz, gcc-4.9, x64), 1000 runs
  + PCC2 20160325               AC: 4.34,   Non-AC: 1.25
  + c2ng 20160326               AC: 8.60,   Non-AC: 2.65
  + cache config in algo        AC: 4.83,   Non-AC: 1.60
  + 20180304 all types int      AC: 4.57,   Non-AC: 1.60
  + 20180304 inlining           AC: 4.35,   Non-AC: 1.42
  + 20180304 precompute specs   AC: 4.24,   Non-AC: 1.42
  + 20190511                    AC: 3.99,   Non-AC: 1.32
  + 20190511 beamFindNearest,   AC; 3.95,   Non-AC: 1.32 */

using util::divideAndRound;

namespace {
    /** Bitmask of VCR capabilities supported by this player. */
    const uint16_t SUPPORTED_CAPABILITIES = game::v3::structures::DeathRayCapability | game::v3::structures::ExperienceCapability | game::v3::structures::BeamCapability;

    /** Movement timer. We check whether standoff distance has been
        reached every this many ticks (it could have been done a little
        simpler, but this way integrates neatly into the timer framework
        we need for regular interval checks). */
    static const int DET_MOVEMENT_TIMER = 100;

    /** Inactivity timer. We check whether combat made progress every this
        many ticks after standoff distance has been reached. A simple
        heuristic increases the value if necessary. */
    static const int DET_INACTIVITY_TIMER = 5000;

    /** Get experience-modified value of an option.
        \param opt primary option
        \param exp experience modificator (EMod) option
        \param min,max range for result */
    int getExperienceModifiedValue(const game::config::HostConfiguration::StandardOption_t& opt,
                                const game::config::HostConfiguration::ExperienceOption_t& exp,
                                const game::vcr::Object& obj,
                                int min,
                                int max)
    {
        int32_t sum = opt(obj.getOwner());
        if (obj.getExperienceLevel() != 0) {
            sum += exp(obj.getExperienceLevel());
        }
        if (sum < min) {
            return min;
        } else if (sum > max) {
            return max;
        } else {
            return sum;
        }
    }
}

/*
 *  Regular (non-alternative) formula
 */

struct game::vcr::classic::PVCRAlgorithm::RegularFormula {
#ifdef PVCR_INTEGER
    /** Compute shield damage, regular combat, integer version. Returns actual value. */
    static inline int32_t computeShieldDamageS(int expl, int kill, const Status& st)
        {
            int32_t damage = divideAndRound(st.f.ShieldDamageScaling * int32_t(expl) + st.f.ShieldKillScaling * int32_t(kill), st.f.mass_plus1) + 1;
            return std::min(damage, 10000);
        }
    /** Compute hull damage, regular combat, integer version. Returns actual value. */
    static inline int32_t computeHullDamageS(int expl, int kill, const Status& st)
        {
            int32_t d = divideAndRound(computeShieldDamageS(expl, kill, st) * st.f.HullDamageScaling, st.f.mass_plus1) + 1;
            return std::min(d, 10000);
        }
    /** Compute killed crew, regular combat, integer version. Returns actual value. */
    static inline int32_t computeCrewKilledS(int kill, bool death, const Status& st)
        {
            int32_t rv = divideAndRound(st.f.CrewKillScaling * int32_t(kill), st.f.mass_plus1);
            if (death && rv == 0) {
                return 1;
            } else {
                return rv;
            }
        }
#else
    /** Compute shield damage, regular combat, float version. Returns actual value. */
    static inline double computeShieldDamage(int expl, int kill, const Status& st)
        {
            double damage = (st.f.ShieldDamageScaling * double(expl) + st.f.ShieldKillScaling * double(kill))
                / double(st.f.mass_plus1);
            if (damage > 10000) {
                damage = 10000;
            }
            return (int) (damage + 1.5);
        }
    /** Compute hull damage, regular combat, float version. Returns actual value. */
    static inline double computeHullDamage(int expl, int kill, const Status& st)
        {
            double d = computeShieldDamage(expl, kill, st) * st.f.HullDamageScaling / double(st.f.mass_plus1);
            if (d > 10000) {
                return 10000;
            } else {
                return (int) (d + 1.5);
            }
        }
    /** Compute killed crew, regular combat, float version. Returns actual value. */
    static inline double computeCrewKilled(int kill, bool death, const Status& st)
        {
            double d = st.f.CrewKillScaling * double(kill) / double(st.f.mass_plus1);
            int i = int(d + 0.5);
            if (death && i == 0) {
                return 1;
            } else {
                return i;
            }
        }
#endif
};

/** Damage formulas for Alternative Combat. */
struct game::vcr::classic::PVCRAlgorithm::AlternativeFormula {
#ifdef PVCR_INTEGER
    /** Compute shield damage, alternative combat, integer version. Returns value sacled by mass+1. */
    static inline int32_t computeShieldDamageS(int expl, int kill, const Status& st)
        {
            int32_t damage = (st.f.ShieldDamageScaling * int32_t(expl) + st.f.ShieldKillScaling * int32_t(kill));
            return std::min(damage, int32_t(st.f.max_scaled));
        }
    /** Compute hull damage, alternative combat, integer version. Returns value scaled by mass+1. */
    static inline int32_t computeHullDamageS(int expl, int /*kill*/, const Status& st)
        {
            int32_t d = int32_t(expl) * st.f.HullDamageScaling;
            return std::min(d, int32_t(st.f.max_scaled));
        }
    /** Compute killed crew, alternative combat, integer version. Returns value scaled by mass+1. */
    static inline int32_t computeCrewKilledS(int kill, bool /*death*/, const Status& st)
        {
            return st.f.CrewKillScaling * int32_t(kill);
        }
#else
    /** Compute shield damage, alternative combat, float version. Returns actual value. */
    static inline double computeShieldDamage(int expl, int kill, const Status& st)
        {
            double damage = (st.f.ShieldDamageScaling * double(expl)
                             + st.f.ShieldKillScaling * double(kill))
                / double(st.f.mass_plus1);
            if (damage > 10000)
                damage = 10000;
            return damage;
        }
    /** Compute hull damage, alternative combat, float version. Returns actual value. */
    static inline double computeHullDamage(int expl, int /*kill*/, const Status& st)
        {
            double d = double(expl) * st.f.HullDamageScaling / double(st.f.mass_plus1);
            if (d > 10000)
                return 10000;
            else
                return d;
        }
    /** Compute killed crew, alternative combat, float version. Returns actual value. */
    static inline double computeCrewKilled(int kill, bool /*death*/, const Status& st)
        {
            return st.f.CrewKillScaling * double(kill) / double(st.f.mass_plus1);
        }
#endif
};





game::vcr::classic::PVCRAlgorithm::PVCRAlgorithm(bool phost3Flag,
                                                 Visualizer& vis,
                                                 const game::config::HostConfiguration& config,
                                                 const game::spec::BeamVector_t& beams,
                                                 const game::spec::TorpedoVector_t& launchers)
    : Algorithm(vis),
      m_config(config),
      m_beams(beams),
      m_launchers(launchers),
      m_phost3Flag(phost3Flag),
      m_seed(0),
      m_time(0),
      m_done(false),
      one_f(0),
      right_probab(0),
      m_capabilities(9),
      det_valid(false),
      det_timer(0),
      m_result(),
      m_alternativeCombat(false),
      m_fireOnAttackFighters(false),
      m_standoffDistance(10000)
{ }

// Destructor.
game::vcr::classic::PVCRAlgorithm::~PVCRAlgorithm()
{
}

// Algorithm methods:

// Check and correct VCR.
bool
game::vcr::classic::PVCRAlgorithm::checkBattle(Object& left, Object& right, uint16_t& /*seed*/)
{
    // ex VcrPlayerPHost::checkVcr
    bool leftResult = checkSide(left);
    bool rightResult = checkSide(right);
    return leftResult || rightResult;
}

// Initialize VCR Player.
void
game::vcr::classic::PVCRAlgorithm::initBattle(const Object& left, const Object& right, uint16_t seed)
{
    // ex VcrPlayerPHost::initVcr
    m_result = BattleResult_t();

    Object leftCopy(left);
    Object rightCopy(right);
    if (checkBattle(leftCopy, rightCopy, seed)) {
        m_result += Invalid;
        m_done = true;
        return;
    }

    // Initialize Playback
    m_time = 0;
    m_seed = uint32_t(seed) << 16;
    m_done = false;
    m_alternativeCombat = m_config[m_config.AllowAlternativeCombat]();
    m_fireOnAttackFighters = m_config[m_config.FireOnAttackFighters]();
    m_standoffDistance = m_config[m_config.StandoffDistance]();

    m_status[LeftSide].r.m_objectX = -29000;
    m_status[RightSide].r.m_objectX = +29000;
    for (int side = 0; side < 2; ++side) {
        Status& st = m_status[side];
        st.f.side = side ? RightSide : LeftSide;
        st.r.obj = side ? rightCopy : leftCopy;
        st.m_statistic = st.r.obj;
#ifdef PVCR_INTEGER
        if (m_alternativeCombat) {
            st.f.scale = st.r.obj.getMass() + 1;
        } else {
            st.f.scale = 1;
        }
        st.r.shield_scaled  = st.r.obj.getShield() * int32_t(st.f.scale);
        st.r.crew_scaled2   = st.r.obj.getCrew() * int32_t(st.f.scale) * 100;
        st.r.damage_scaled2 = st.r.obj.getDamage() * int32_t(st.f.scale) * 100;
        st.f.max_scaled     = int32_t(10000) * st.f.scale;
        st.f.mass_plus1     = st.r.obj.getMass() + 1;
#else
        st.r.shield     = st.r.obj.getShield();
        st.r.crew       = st.r.obj.getCrew();
        st.r.damage     = st.r.obj.getDamage();
        st.f.mass_plus1 = st.r.obj.getMass() + 1;
#endif

        if (st.r.obj.getNumBeams() > 0) {
            if (const game::spec::Beam* b = m_beams.get(st.r.obj.getBeamType())) {
                st.f.beam_hit_odds = computeBeamHitOdds     (*b, st.r.obj);
                st.f.beam_recharge = computeBeamRechargeRate(*b, st.r.obj);
                st.f.beam_kill = b ? b->getKillPower() : 0;
                st.f.beam_damage = b ? b->getDamagePower() : 0;
            } else {
                st.f.beam_hit_odds = 0;
                st.f.beam_recharge = 1;
                st.f.beam_kill     = 0;
                st.f.beam_damage   = 0;
            }
        } else {
            st.f.beam_hit_odds = 0;
            st.f.beam_recharge = 1;
            st.f.beam_kill     = 0;
            st.f.beam_damage   = 0;
        }
        if (st.r.obj.getNumLaunchers() > 0) {
            if (const game::spec::TorpedoLauncher* t = m_launchers.get(st.r.obj.getTorpedoType())) {
                st.f.torp_hit_odds = computeTorpHitOdds(*t, st.r.obj);
                st.f.torp_recharge = computeTubeRechargeRate(*t, st.r.obj);
                st.f.torp_kill     = t->getKillPower();
                st.f.torp_damage   = t->getDamagePower();
            } else {
                st.f.torp_hit_odds = 0;
                st.f.torp_recharge = 1;
                st.f.torp_kill     = 0;
                st.f.torp_damage   = 0;
            }
            if (!m_config[m_config.AllowAlternativeCombat]()) {
                st.f.torp_kill   *= 2;
                st.f.torp_damage *= 2;
            }
        } else {
            st.f.torp_hit_odds = 0;
            st.f.torp_recharge = 1;
            st.f.torp_kill     = 0;
            st.f.torp_damage   = 0;
        }
        if (st.r.obj.getNumBays() > 0) {
            st.f.bay_recharge = computeBayRechargeRate(st.r.obj.getNumBays(), st.r.obj);
        } else {
            st.f.bay_recharge = 0;
        }

//         // FIXME: this is still missing, but since the original
//         // line is bogus (should be p.Shield, not VCR.Shield) and it works,
//         // it's probably not needed.
//         //   IF NOT IsPlanet(Who) AND IsFreighter(Who) THEN VCR.Shield[Who]:=0;
//         // Change line below if needed.

        int charge = (st.r.obj.getShield() == 100) ? 1000 : 0;
        std::fill_n(st.r.m_beamStatus, int(VCR_MAX_BEAMS), charge);
        std::fill_n(st.r.m_launcherStatus, int(VCR_MAX_TORPS), charge);
        std::fill_n(st.r.m_bayStatus,  int(VCR_MAX_BAYS),  0); // !!!
        std::fill_n(st.r.m_fighterStatus, int(VCR_MAX_FTRS), uint8_t(FighterIdle));
        std::fill_n(st.r.m_fighterX, int(VCR_MAX_FTRS), 0);  // not needed?
        std::fill_n(st.r.m_fighterStrikesLeft, int(VCR_MAX_FTRS), 0);  // not needed?
        st.r.m_launchCountdown = st.r.m_activeFighters = 0;

#ifdef PVCR_INTEGER
        /* Original comparison was:
            if (st.r.damage_scaled2 + 50 * st.f.scale >=
                st.f.damage_limit * 100 * st.f.scale)          */
        st.f.damage_limit_scaled =
            m_config.getPlayerRaceNumber(st.r.obj.getOwner()) == 2
            ? (150 * 2 - 1) * 50 * st.f.scale
            : (100 * 2 - 1) * 50 * st.f.scale;
#else
        st.f.damage_limit = (m_config.getPlayerRaceNumber(st.r.obj.getOwner()) == 2 ? 150 : 100);
#endif

        const int owner = st.r.obj.getOwner();
        st.f.ShieldDamageScaling  = getExperienceModifiedValue(m_config[m_config.ShieldDamageScaling],  m_config[m_config.EModShieldDamageScaling],  st.r.obj, 0, 32767);
        st.f.ShieldKillScaling    = getExperienceModifiedValue(m_config[m_config.ShieldKillScaling],    m_config[m_config.EModShieldKillScaling],    st.r.obj, 0, 32767);
        st.f.HullDamageScaling    = getExperienceModifiedValue(m_config[m_config.HullDamageScaling],    m_config[m_config.EModHullDamageScaling],    st.r.obj, 0, 32767);
        st.f.MaxFightersLaunched  = getExperienceModifiedValue(m_config[m_config.MaxFightersLaunched],  m_config[m_config.EModMaxFightersLaunched],  st.r.obj, 0, VCR_MAX_FTRS);
        st.f.StrikesPerFighter    = getExperienceModifiedValue(m_config[m_config.StrikesPerFighter],    m_config[m_config.EModStrikesPerFighter],    st.r.obj, 1, 100);
        st.f.BayLaunchInterval    = m_config[m_config.BayLaunchInterval](owner);
        st.f.FighterMovementSpeed = getExperienceModifiedValue(m_config[m_config.FighterMovementSpeed], m_config[m_config.EModFighterMovementSpeed], st.r.obj, 1, 10000);
        st.f.FighterBeamExplosive = getExperienceModifiedValue(m_config[m_config.FighterBeamExplosive], m_config[m_config.EModFighterBeamExplosive], st.r.obj, 1, 1000);
        st.f.FighterBeamKill      = getExperienceModifiedValue(m_config[m_config.FighterBeamKill],      m_config[m_config.EModFighterBeamKill],      st.r.obj, 1, 1000);
        st.f.FighterFiringRange   = m_config[m_config.FighterFiringRange](owner);
        st.f.BeamHitFighterRange  = m_config[m_config.BeamHitFighterRange](owner);
        st.f.BeamHitFighterCharge = getExperienceModifiedValue(m_config[m_config.BeamHitFighterCharge], m_config[m_config.EModBeamHitFighterCharge], st.r.obj, 1, 1000);
        st.f.BeamFiringRange      = m_config[m_config.BeamFiringRange](owner);
        st.f.BeamHitShipCharge    = m_config[m_config.BeamHitShipCharge](owner);
        st.f.TorpFiringRange      = m_config[m_config.TorpFiringRange](owner);
        st.f.ShipMovementSpeed    = m_config[m_config.ShipMovementSpeed](owner);

        st.f.CrewKillScaling =
            divideAndRound((100-st.r.obj.getCrewDefenseRate()) * getExperienceModifiedValue(m_config[m_config.CrewKillScaling], m_config[m_config.EModCrewKillScaling], st.r.obj, 0, 32767),
                  100);
    }

    // pre-compute fighter intercept probabilities
    const game::config::HostConfiguration::StandardOption_t& FighterKillOdds = m_config[m_config.FighterKillOdds];
    if (m_phost3Flag) {
        // PHost 3 or 4
        int left_odds  = FighterKillOdds(m_status[LeftSide].r.obj.getOwner());
        int right_odds = FighterKillOdds(m_status[RightSide].r.obj.getOwner());
        int left_f   = (100 - left_odds) * right_odds;
        int right_f  = (100 - right_odds) * left_odds;
        one_f        = (left_f + right_f) / 100;
        if (one_f == 0)
            right_probab = 50;
        else
            right_probab = right_f / one_f;
        // FIXME: battle.c seems to do `right_probab = left_f / one_f'
    } else {
        // In PHost 2, combat options were not arrayized.
        // Hence, for a valid pconfig, all FighterKillOdds values are the same and we can pick any one
        one_f        = FighterKillOdds(1);
        right_probab = 50;
    }

    initActivityDetector();
}


// Finish up VCR.
void
game::vcr::classic::PVCRAlgorithm::doneBattle(Object& left, Object& right)
{
    // ex VcrPlayerPHost::doneVcr
//     ASSERT(status_word != VCRS_INVALID);
//     ASSERT(done);

    for (int side = 0; side < 2; ++side) {
        Status& st = m_status[side];
#ifdef PVCR_INTEGER
        if (m_alternativeCombat) {
            st.r.obj.setDamage(divideAndRound(st.r.damage_scaled2, st.f.scale * 100));
            st.r.obj.setCrew  (divideAndRound(st.r.crew_scaled2,   st.f.scale * 100));
            st.r.obj.setShield(divideAndRound(st.r.shield_scaled,  st.f.scale));
        } else {
            st.r.obj.setDamage(divideAndRound(st.r.damage_scaled2, 100));
            st.r.obj.setCrew  (divideAndRound(st.r.crew_scaled2, 100));
            st.r.obj.setShield(st.r.shield_scaled);
        }
#else
        st.r.obj.setDamage(int(st.r.damage + 0.5));
        st.r.obj.setCrew  (int(st.r.crew + 0.5));
        st.r.obj.setShield(int(st.r.shield + 0.5));
#endif
        if (st.r.obj.getDamage() > 100)   // FIXME: take out?
            st.r.obj.setDamage(100);

#ifdef PVCR_INTEGER
        if (canStillFight(st, m_status[!side]) && st.r.obj.getDamage() < 100)
#else
        if (canStillFight(st, m_status[!side]) && st.r.damage < 99.5)
#endif
        {
            for (int i = 0; i < st.f.MaxFightersLaunched; ++i) {
                if (st.r.m_fighterStatus[i] != FighterIdle) {
                    st.r.obj.addFighters(+1);
                    --st.r.m_activeFighters;
                    visualizer().landFighter(*this, st.f.side, i);
                    st.r.m_fighterStatus[i] = FighterIdle;
                }
            }
            // ASSERT(st.r.m_activeFighters == 0);
        }
        st.r.m_activeFighters = 0;
    }

    m_result = BattleResult_t();

    /* Lizards fight up to 150, but explode afterwards. */
    if (m_status[LeftSide].r.obj.getDamage() >= 100)
        m_result += LeftDestroyed;
    else if (m_status[LeftSide].r.obj.getCrew() <= 0)
        m_result += LeftCaptured;

    if (m_status[RightSide].r.obj.getDamage() >= 100)
        m_result += RightDestroyed;
    else if (!m_status[RightSide].r.obj.isPlanet() && m_status[RightSide].r.obj.getCrew() <= 0)
        m_result += RightCaptured;

    if (m_result.contains(LeftDestroyed))
        visualizer().killObject(*this, LeftSide);
    if (m_result.contains(RightDestroyed))
        visualizer().killObject(*this, RightSide);

    if (m_result.empty())
        /* FIXME: can we guarantee that every status not caught by the above is a stalemate? */
        m_result += Stalemate;

    left = m_status[LeftSide].r.obj;
    right = m_status[RightSide].r.obj;
}

bool
game::vcr::classic::PVCRAlgorithm::setCapabilities(uint16_t cap)
{
    // ex VcrPlayerPHost::setCapabilities
    if ((cap & ~SUPPORTED_CAPABILITIES) != 0) {
        return false;
    }
    m_capabilities = cap;
    return true;
}

// Play one cycle.
bool
game::vcr::classic::PVCRAlgorithm::playCycle()
{
    // ex VcrPlayerPHost::playCycle
    if (!canStillFight(m_status[LeftSide], m_status[RightSide]) && !canStillFight(m_status[RightSide], m_status[LeftSide]))
        m_done = true;
    else if (!checkCombatActivity())
        m_done = true;

    if (m_done)
        return false;

    ++m_time;
    if (m_status[LeftSide].r.m_launchCountdown > 0)
        --m_status[LeftSide].r.m_launchCountdown;
    if (m_status[RightSide].r.m_launchCountdown > 0)
        --m_status[RightSide].r.m_launchCountdown;

    fighterRecharge(m_status[LeftSide]);
    fighterRecharge(m_status[RightSide]);
    beamRecharge(m_status[LeftSide]);
    beamRecharge(m_status[RightSide]);
    torpsRecharge(m_status[LeftSide]);
    torpsRecharge(m_status[RightSide]);
    fighterLaunch(m_status[LeftSide]);
    fighterLaunch(m_status[RightSide]);

    if (fighterAttack(m_status[LeftSide], m_status[RightSide]) || fighterAttack(m_status[RightSide], m_status[LeftSide])
        || torpsFire(m_status[LeftSide], m_status[RightSide]) || torpsFire(m_status[RightSide], m_status[LeftSide])
        || beamFire(m_status[LeftSide], m_status[RightSide]) || beamFire(m_status[RightSide], m_status[LeftSide]))
    {
        m_done = true;
    } else {
        fighterIntercept();
        fighterMove(m_status[LeftSide]);
        fighterMove(m_status[RightSide]);
        moveObjects();
    }
    return true;
}

// Fast forward.
void
game::vcr::classic::PVCRAlgorithm::playFastForward()
{
    // ex VcrPlayerPHost::playFastForward
    if (m_time == 0
        && m_status[LeftSide].r.obj.getNumBays() == 0 && m_status[RightSide].r.obj.getNumBays() == 0
        && m_status[LeftSide].r.obj.getShield() == 100 && m_status[RightSide].r.obj.getShield() == 100)
    {
        /* no carrier in play, both fully charged. Hence, no recharge
           work to be done. First interesting thing will happen when
           we are in beam range. PHost still polls the PRNG once for
           every beam and tick, as long as beams are charged above
           BeamHitFighterCharge. */
        /* FIXME: same applies if one ship is a freighter */
        int32_t dist_now = getDistance();
        int32_t target_dist = m_standoffDistance;
        if (target_dist < m_status[LeftSide].f.BeamFiringRange)
            target_dist = m_status[LeftSide].f.BeamFiringRange;
        if (target_dist < m_status[RightSide].f.BeamFiringRange)
            target_dist = m_status[RightSide].f.BeamFiringRange;
        if (m_status[LeftSide].r.obj.getNumLaunchers() > 0)
            if (target_dist < m_status[LeftSide].f.TorpFiringRange)
                target_dist = m_status[LeftSide].f.TorpFiringRange;
        if (m_status[RightSide].r.obj.getNumLaunchers() > 0)
            if (target_dist < m_status[RightSide].f.TorpFiringRange)
                target_dist = m_status[RightSide].f.TorpFiringRange;

        int speed = m_status[LeftSide].f.ShipMovementSpeed;
        if (!m_status[RightSide].r.obj.isPlanet())
            speed += m_status[RightSide].f.ShipMovementSpeed;

        if (target_dist < dist_now && speed > 0) {
            /* We'll move towards each other at a nonzero speed */
            int adv_time = (dist_now - target_dist) / speed;
            if (adv_time > 2) {
                /* less 2 ticks, for safety */
                adv_time -= 2;

                /* advance time */
                m_time += adv_time;
                m_status[LeftSide].r.m_objectX += m_status[LeftSide].f.ShipMovementSpeed * adv_time;
                if (!m_status[RightSide].r.obj.isPlanet())
                    m_status[RightSide].r.m_objectX -= m_status[RightSide].f.ShipMovementSpeed * adv_time;

                /* advance PRNG. Although there is a formula how to
                   advance a linear congruence *fast*, we cannot use
                   it because randomRange() advances the seed twice in
                   36 out of 65536 cases. */
                /* For the simulator, a more adventurous FF scheme
                   would be to *not* advance the PRNG, and calculate
                   the initial seed backwards when the user wants to
                   see this instance. */
                /* The maximum num_pulls is around 10000 for regular
                   configurations, so there's no point in optimizing
                   for >65500. */
                int num_pulls = 0;
                if (m_status[LeftSide].f.BeamHitFighterCharge <= 1000)
                    num_pulls += m_status[LeftSide].r.obj.getNumBeams();
                if (m_status[RightSide].f.BeamHitFighterCharge <= 1000)
                    num_pulls += m_status[RightSide].r.obj.getNumBeams();
                num_pulls *= adv_time;
                while (num_pulls-- > 0)
                    randomRange100();
            }
        }
    }
}

int
game::vcr::classic::PVCRAlgorithm::getBeamStatus(Side side, int id)
{
    // ex VcrPlayerPHost::getBeamStatus
    return m_status[side].r.m_beamStatus[id] / 10;
}

int
game::vcr::classic::PVCRAlgorithm::getLauncherStatus(Side side, int id)
{
    // ex VcrPlayerPHost::getTorpStatus
    return m_status[side].r.m_launcherStatus[id] / 10;
}

int
game::vcr::classic::PVCRAlgorithm::getNumTorpedoes(Side side)
{
    // ex VcrPlayerPHost::getTorpCount
    return m_status[side].r.obj.getNumTorpedoes();
}

int
game::vcr::classic::PVCRAlgorithm::getNumFighters(Side side)
{
    // ex VcrPlayerPHost::getFighterCount
    return m_status[side].r.obj.getNumFighters();
}

int
game::vcr::classic::PVCRAlgorithm::getShield(Side side)
{
    // ex VcrPlayerPHost::getShields
    // Round, because driver code also rounds.
#ifdef PVCR_INTEGER
    return divideAndRound(m_status[side].r.shield_scaled, m_status[side].f.scale);
#else
    return int(m_status[side].r.shield + 0.5);
#endif
}

int
game::vcr::classic::PVCRAlgorithm::getDamage(Side side)
{
    // ex VcrPlayerPHost::getDamage
#ifdef PVCR_INTEGER
    return divideAndRound(m_status[side].r.damage_scaled2, m_status[side].f.scale * 100);
#else
    return int(m_status[side].r.damage + 0.5);
#endif
}

int
game::vcr::classic::PVCRAlgorithm::getCrew(Side side)
{
    // ex VcrPlayerPHost::getCrew
#ifdef PVCR_INTEGER
    return divideAndRound(m_status[side].r.crew_scaled2, m_status[side].f.scale * 100);
#else
    return int(m_status[side].r.crew + 0.5);
#endif
}

int
game::vcr::classic::PVCRAlgorithm::getFighterX(Side side, int id)
{
    // ex VcrPlayerPHost::getFighterX
    // Coordinate range is approx. [-29000,29000], i.e. about int16_t range.
    // We map that to ~[4%,96%] MAX_COORDINATE, or [37,603] for MAX_COORDINATE=640.
    // HostAlgorithm has [30,610] by default.
    return (m_status[side].r.m_fighterX[id] * MAX_COORDINATE / 65536) + (MAX_COORDINATE/2);
}

game::vcr::classic::FighterStatus
game::vcr::classic::PVCRAlgorithm::getFighterStatus(Side side, int id)
{
    // ex VcrPlayerPHost::getFighterStatus
    return FighterStatus(m_status[side].r.m_fighterStatus[id]);
}

int
game::vcr::classic::PVCRAlgorithm::getObjectX(Side side)
{
    // ex VcrPlayerPHost::getObjectX
    return (m_status[side].r.m_objectX * MAX_COORDINATE / 65536) + (MAX_COORDINATE/2);
}

int32_t
game::vcr::classic::PVCRAlgorithm::getDistance()
{
    // ex VcrPlayerPHost::getDistance
    return m_status[RightSide].r.m_objectX - m_status[LeftSide].r.m_objectX;
}

// FIXME: move
struct game::vcr::classic::PVCRAlgorithm::PVCRStatusToken : public StatusToken {
    Status m_status[2];
    int32_t m_seed;
    BattleResult_t m_result;
    bool m_done;
    int one_f, right_probab;
    PVCRStatusToken(Time_t t)
        : StatusToken(t)
        { }
};

// Save status.
// Easy mindless way, just save everything.
// Actually, we could re-compute status[].f and one_f/right_probab from the VCR record, but I'm too lazy to do that now.
game::vcr::classic::StatusToken*
game::vcr::classic::PVCRAlgorithm::createStatusToken()
{
    // ex VcrPlayerPHost::getStatusToken()
    PVCRStatusToken* s = new PVCRStatusToken(m_time);

    s->m_status[LeftSide]    = m_status[LeftSide];
    s->m_status[RightSide]    = m_status[RightSide];
    s->m_seed       = m_seed;
    s->m_result     = m_result;
    s->m_done       = m_done;
    s->one_f        = one_f;
    s->right_probab = right_probab;

    return s;
}

void
game::vcr::classic::PVCRAlgorithm::restoreStatus(const StatusToken& token)
{
    // ex VcrPlayerPHost::setStatus
    if (const PVCRStatusToken* t = dynamic_cast<const PVCRStatusToken*>(&token)) {
        m_status[LeftSide]    = t->m_status[LeftSide];
        m_status[RightSide]    = t->m_status[RightSide];
        m_seed       = t->m_seed;
        m_result     = t->m_result;
        m_done       = t->m_done;
        one_f        = t->one_f;
        right_probab = t->right_probab;
        m_time       = t->getTime();
    }
}

game::vcr::classic::Time_t
game::vcr::classic::PVCRAlgorithm::getTime()
{
    return m_time;
}

game::vcr::classic::BattleResult_t
game::vcr::classic::PVCRAlgorithm::getResult()
{
    return m_result;
}

game::vcr::Statistic
game::vcr::classic::PVCRAlgorithm::getStatistic(Side side)
{
    return m_status[side].m_statistic;
}


/*
 *  Random Numbers
 *
 *  The PRNG has been optimized to generate the best possible x86 machine
 *  code with my compiler.
 *  - scale everything by 65536; saves one masking operation
 *  - "a+b*-c" instead of "a-b*c"
 */

/** Random Number Generator. Basic linear congruence. */
uint32_t
game::vcr::classic::PVCRAlgorithm::random64k()
{
    // ex VcrPlayerPHost::random64k
    m_seed = (13489*65536 + m_seed * -259U);
    return m_seed;
}

#ifdef PVCR_PREPARED_RNG
/** Random Number Generator.
    \return random number uniformly in [0,max), where max is the parameter passed to PreparedRNG::operator=. */
int
game::vcr::classic::PVCRAlgorithm::randomRange(const PreparedRNG& rng)
{
    // ex VcrPlayerPHost::randomRange
    uint32_t max = rng.limit;
    uint32_t i;
    do {
        i = random64k();
    } while (i >= max);
    return i / rng.divi;
}

/** Prepare random number generator.
    The PreparedRNG can later be passed to randomRange to return a number in [0,max).
    Doing it this way avoids one division instruction (or more) per RNG invocation. */
void
game::vcr::classic::PVCRAlgorithm::PreparedRNG::operator=(uint32_t max)
{
    // ex VcrPlayerPHost::PreparedRNG::operator=
    if (max == 0) {
        // Pathological case. These RNGs will never be called.
        divi = 65536U*65535;
        limit = 65536U*65535;
    } else {
        divi  = (65535U / max) * 65536;
        limit = divi * max;
    }
}
#else
/** Random Number Generator.
    \return random number uniformly in [0, max) */
int
game::vcr::classic::PVCRAlgorithm::randomRange(uint32_t max)
{
    // ex VcrPlayerPHost::randomRange
    uint32_t divisor = (65535U / max) * 65536;
    uint32_t i;
    do {
        i = random64k() / divisor;
    } while (i >= max);
    return i;
}
#endif

/** Random Number Generator.
    Same as randomRange(100), optimized to avoid divisions. */
int
game::vcr::classic::PVCRAlgorithm::randomRange100()
{
    // ex VcrPlayerPHost::randomRange100
    uint32_t i;
    do {
        i = random64k();
    } while (i >= 65500U*65536U);
    return i / (65536U*655U);
}

/** Random Number Generator.
    Compute a random number uniformly in [0,100) and return true iff it is smaller than comp. */
bool
game::vcr::classic::PVCRAlgorithm::randomRange100LT(int comp)
{
    // ex VcrPlayerPHost::randomRange100LT
    uint32_t i;
    do {
        i = random64k();
    } while (i >= 65500U*65536U);

    // comp could be >100, which would make 65536*655*comp overflow.
    // Add one extra shift instruction instead.
    i >>= 16;
    return i < uint32_t(comp * 655);
}


/*
 *  Hit
 */

/** Hit object, back-end.
    \tparam Formula formula set in use (AlternativeFormula, RegularFormula).
    \param st    object being hit
    \param kill  anti-life (x-ray) power of weapon
    \param expl  explosive power of weapon
    \param is_death_ray true iff this weapon emits death rays.
    \retval true battle ends
    \retval false battle continues */
template<typename Formula>
inline bool
game::vcr::classic::PVCRAlgorithm::hitT(Status& st, int kill, int expl, bool is_death_ray)
{
    // ex VcrPlayerPHost::hitT
#ifdef PVCR_INTEGER
    int damage_rate = 100;

    if (!is_death_ray) {
        /* Hit shields */
        if (st.r.shield_scaled > 0) {
            int32_t damage_s = Formula::computeShieldDamageS(expl, kill, st);
            if (st.r.shield_scaled < damage_s) {
                // Shields are completely worn down by this hit
                damage_rate = (damage_s - st.r.shield_scaled) * 100 / damage_s;
                st.r.shield_scaled = 0;
            } else {
                // Shields still hold
                damage_rate = 0;
                st.r.shield_scaled -= damage_s;
            }
            if (damage_rate <= 0)
                return false;
        }

        /* Shields are down -- do damage */
        st.r.damage_scaled2 += Formula::computeHullDamageS(expl, kill, st) * damage_rate;
        if (st.r.damage_scaled2 >= st.f.damage_limit_scaled) {
            return true;
        }
    }

    if (!st.r.obj.isPlanet()) {
        int32_t killed_s = Formula::computeCrewKilledS(kill, is_death_ray, st) * damage_rate;
        st.r.crew_scaled2 -= killed_s;
        if (st.r.crew_scaled2 < 50*st.f.scale) {
            st.r.crew_scaled2 = 0;
            return true;
        }
    }

    return false;
#else
    int damage_rate = 100;

    if (!is_death_ray) {
        /* Hit shields */
        if (st.r.shield > 0) {
            double damage = Formula::computeShieldDamage(expl, kill, st);
            if (st.r.shield <= damage) {
                // Shields are completely worn down by this hit
                damage_rate = int((damage - st.r.shield) * 100 / damage);
                st.r.shield = 0;
            } else {
                // Shields still hold
                damage_rate = 0;
                st.r.shield -= damage;
            }
            if (damage_rate <= 0)
                return false;
        }

        /* Shields are down -- do damage */
        st.r.damage += Formula::computeHullDamage(expl, kill, st) * damage_rate / 100.0;

        if (int(st.r.damage + 0.5) >= st.f.damage_limit) {
            return true;
        }
    }

    if (!st.r.obj.isPlanet()) {
        double killed = Formula::computeCrewKilled(kill, is_death_ray, st) * damage_rate / 100.0;
        st.r.crew -= killed;
        if (st.r.crew < 0.5) {
            st.r.crew = 0;
            return true;
        }
    }
    return false;
#endif
}

/** Hit object.
    \param st    object being hit
    \param kill  anti-life (x-ray) power of weapon
    \param expl  explosive power of weapon
    \param is_death_ray true iff this weapon emits death rays.
    \retval true battle ends
    \retval false battle continues */
bool
game::vcr::classic::PVCRAlgorithm::hit(Status& st, int kill, int expl, bool is_death_ray)
{
    // ex VcrPlayerPHost::hit
    if (kill <= 0) {
        kill = 1;
    }
    if (expl <= 0) {
        expl = 1;
    }
    if ((m_capabilities & game::v3::structures::DeathRayCapability) == 0) {
        is_death_ray = false;
    }

    if (m_alternativeCombat) {
        return hitT<AlternativeFormula>(st, kill, expl, is_death_ray);
    } else {
        return hitT<RegularFormula>(st, kill, expl, is_death_ray);
    }
}


/*
 *  Fighters
 */

/** Compute bay recharge rate. Documented formula. Used for initialisation. */
int
game::vcr::classic::PVCRAlgorithm::computeBayRechargeRate(int num, const Object& obj) const
{
    // ex VcrPlayerPHost::computeBayRechargeRate
    int i = getExperienceModifiedValue(m_config[m_config.BayRechargeBonus], m_config[m_config.EModBayRechargeBonus], obj, -500, 500) * num
        + getExperienceModifiedValue(m_config[m_config.BayRechargeRate], m_config[m_config.EModBayRechargeRate], obj, 0, 16384);
    return i > 1 ? i : 1;
}

/** Recharge Fighter Bays. */
inline void
game::vcr::classic::PVCRAlgorithm::fighterRecharge(Status& st)
{
    // ex VcrPlayerPHost::fighterRecharge
    register const int mx = st.r.obj.getNumBays();
    for (int i = 0; i < mx; ++i)
        if (st.r.m_bayStatus[i] < 1000)
            st.r.m_bayStatus[i] += randomRange(st.f.bay_recharge);
}

/** Launch Fighters. */
inline void
game::vcr::classic::PVCRAlgorithm::fighterLaunch(Status& st)
{
    // ex VcrPlayerPHost::fighterLaunch
    /* can we launch a fighter? */
    if (st.r.obj.getNumFighters() == 0
        || st.r.m_activeFighters >= st.f.MaxFightersLaunched
        || st.r.m_launchCountdown > 0)
        return;

    /* yes, we can! */
    int bay_mx = st.r.obj.getNumBays();
    for (int bay = 0; bay < bay_mx; ++bay) {
        if (st.r.m_bayStatus[bay] >= 1000) {
            for (int track = 0; track < st.f.MaxFightersLaunched; ++track) {
                if (st.r.m_fighterStatus[track] == FighterIdle) {
                    /* okay, we have a bay which is ready, and an empty track. */
                    st.r.m_fighterStatus[track]     = FighterAttacks;
                    st.r.m_fighterX[track]          = st.r.m_objectX;
                    st.r.m_fighterStrikesLeft[track] = uint16_t(st.f.StrikesPerFighter);
                    st.r.m_bayStatus[bay]           = 0;
                    st.r.m_activeFighters++;
                    st.r.obj.addFighters(-1);
                    st.r.m_launchCountdown = st.f.BayLaunchInterval;
                    visualizer().startFighter(*this, st.f.side, track);
                    st.m_statistic.handleFightersAboard(st.r.obj.getNumFighters());
                    return;
                }
            }
        }
    }
}

/** Move Fighters. Takes back returned fighters. */
inline void
game::vcr::classic::PVCRAlgorithm::fighterMove(Status& st)
{
    // ex VcrPlayerPHost::fighterMove
    if (st.r.m_activeFighters == 0)
        return;

    Side side = st.f.side;
    int dir = (side == LeftSide) ? st.f.FighterMovementSpeed : -st.f.FighterMovementSpeed;

    for (int track = 0, limit = st.f.MaxFightersLaunched; track < limit; ++track)
        if (st.r.m_fighterStatus[track] == FighterAttacks) {
            st.r.m_fighterX[track] += dir;
        } else if (st.r.m_fighterStatus[track] == FighterReturns) {
            st.r.m_fighterX[track] -= dir;
            if ((side == LeftSide)
                ? st.r.m_fighterX[track] < st.r.m_objectX
                : st.r.m_fighterX[track] > st.r.m_objectX)
            {
                /* fighter comes back to baseship */
                st.r.m_activeFighters--;
                st.r.obj.addFighters(+1);
                visualizer().landFighter(*this, side, track);
                st.r.m_fighterStatus[track] = FighterIdle;
            }
        }
}

/** Do fighter intercepts. */
void
game::vcr::classic::PVCRAlgorithm::fighterIntercept()
{
    // ex VcrPlayerPHost::fighterIntercept
    static const int NEVER = -0x4000;

    /* Fighter intercept only happens if both have fighters */
    if (m_status[LeftSide].r.m_activeFighters == 0 || m_status[RightSide].r.m_activeFighters == 0)
        return;

    /* Compute screen positions. The actual fighter intercept code compares fighter
       positions, with coarse granularity, because exact matches never happen. By
       ignoring the lowest 7 bits of the X coordinate, we effectively compare just
       the upper 9 bits. Fighter intercept processes all fighter pairs which is very
       expensive. We use the following optimisations:
       - precompute the fighter positions (this optimisation is also in PVCR)
       - process only occupied slots
       - process only situations where we know to have fighter pairs. Since we have
         just 9 bits that specify a position, we can easily keep track of which
         points have a left fighter, and only process right fighters at these points.
         Note that eliminating left fighters does not work even if we know that they
         have no counterpart because we must call randomRange() an appropriate number
         of times. */
    int16_t lmatch[VCR_MAX_FTRS], rmatch[VCR_MAX_FTRS];
    int8_t  lslot[VCR_MAX_FTRS],  rslot[VCR_MAX_FTRS];
    int     lcount = 0,           rcount = 0;

    uint32_t bins[16];  /* That's 512 bits */
    for (int i = 0; i < 16; ++i)
        bins[i] = 0;

    /* Compute left positions and mark occupied slots */
    for (int i = 0, limit = m_status[LeftSide].f.MaxFightersLaunched; i < limit; ++i) {
        if (m_status[LeftSide].r.m_fighterStatus[i] != FighterIdle) {
            int hash = m_status[LeftSide].r.m_fighterX[i] >> 7;
            lslot[lcount] = int8_t(i);
            lmatch[lcount] = int16_t(hash);
            lcount++;
            bins[(hash >> 5) & 15] |= 1 << (hash & 31);
        }
    }

    /* Compute right positions, but eliminate impossible slots */
    for (int i = 0, limit = m_status[RightSide].f.MaxFightersLaunched; i < limit; ++i) {
        if (m_status[RightSide].r.m_fighterStatus[i] != FighterIdle) {
            int hash = m_status[RightSide].r.m_fighterX[i] >> 7;
            if (bins[(hash >> 5) & 15] & (1 << (hash & 31))) {
                rslot[rcount] = int8_t(i);
                rmatch[rcount] = int16_t(hash);
                rcount++;
            }
        }
    }

    /* Degenerate case: no match */
    if (!rcount) {
        for (int ls = 0; ls < lcount; ++ls)
            randomRange100();
        return;
    }

    /* Full version */
    for (int ls = 0; ls < lcount; ++ls) {
        if (randomRange100LT(one_f)) {
            for (int rs = 0; rs < rcount; ++rs) {
                if (rmatch[rs] == lmatch[ls]) {
                    int lf = lslot[ls];
                    int rf = rslot[rs];
                    /* two fighters at the same place, intercepting */
                    /* theoretically, here is a slight imbalance. If
                       the "right" fighter survives, it can fire again
                       while a "left" one can not. Whether this is
                       relevant in practice is unknown. */
                    if (randomRange100LT(right_probab)) {
                        visualizer().fireBeam(*this, RightSide, rf, lf, 1, m_status[RightSide].f.FighterBeamExplosive, m_status[RightSide].f.FighterBeamKill);
                        m_status[LeftSide].r.m_activeFighters--;
                        visualizer().killFighter(*this, LeftSide, lf);
                        m_status[LeftSide].r.m_fighterStatus[lf] = FighterIdle;
                    } else {
                        visualizer().fireBeam(*this, LeftSide, lf, rf, 1, m_status[LeftSide].f.FighterBeamExplosive, m_status[LeftSide].f.FighterBeamKill);
                        m_status[RightSide].r.m_activeFighters--;
                        visualizer().killFighter(*this, RightSide, rf);
                        m_status[RightSide].r.m_fighterStatus[rf] = FighterIdle;
                        rmatch[rs] = NEVER;
                    }
                    break;
                }
            }
        }
    }
}

/** Fighters attack enemy. */
inline bool
game::vcr::classic::PVCRAlgorithm::fighterAttack(Status& st, Status& opp)
{
    // ex VcrPlayerPHost::fighterAttack
    /* only if we have fighters */
    if (st.r.m_activeFighters == 0)
        return false;

    int enemy_x = opp.r.m_objectX;
    for (int i = 0, limit = st.f.MaxFightersLaunched; i < limit; ++i) {
        if (st.r.m_fighterStatus[i] == FighterAttacks) {
            if (std::abs(int32_t(st.r.m_fighterX[i]) - enemy_x) <= st.f.FighterFiringRange) {
                st.r.m_fighterStrikesLeft[i]--;
                if (!st.r.m_fighterStrikesLeft[i]) {
                    st.r.m_fighterStatus[i] = FighterReturns;
                }

                bool hitres = hit(opp, st.f.FighterBeamKill, st.f.FighterBeamExplosive, false);
                visualizer().fireBeam(*this, st.f.side, i, -1, 1, st.f.FighterBeamExplosive, st.f.FighterBeamKill);
                if (hitres)
                    return true;
            } else if ((m_capabilities & game::v3::structures::BeamCapability) != 0) {
                /* if fighter moved past enemy, retreat */
                if (st.f.side == 0
                    ? st.r.m_fighterX[i] > enemy_x + st.f.FighterFiringRange
                    : st.r.m_fighterX[i] < enemy_x - st.f.FighterFiringRange)
                {
                    st.r.m_fighterStrikesLeft[i] = 0;
                    st.r.m_fighterStatus[i] = FighterReturns;
                }
            }
        }
    }
    return false;
}


/*
 *  Beams
 */


/** Compute beam hit odds. Documented formula. Used during initialisation. */
int
game::vcr::classic::PVCRAlgorithm::computeBeamHitOdds(const game::spec::Beam& beam, const Object& obj) const
{
    // ex VcrPlayerPHost::computeBeamHitOdds
    int i = getExperienceModifiedValue(m_config[m_config.BeamHitBonus], m_config[m_config.EModBeamHitBonus], obj, -4095, 4095)
        * (beam.getKillPower() + beam.getDamagePower()) / 100
        + getExperienceModifiedValue(m_config[m_config.BeamHitOdds], m_config[m_config.EModBeamHitOdds], obj, 0, 100);
    return i < 0 ? 0 : i;
}

/** Compute beam recharge rate. Documented formula. Used during
    initialisation. */
int
game::vcr::classic::PVCRAlgorithm::computeBeamRechargeRate(const game::spec::Beam& beam, const Object& obj) const
{
    // ex VcrPlayerPHost::computeBeamRechargeRate
    int i = (((beam.getKillPower() + beam.getDamagePower()) * getExperienceModifiedValue(m_config[m_config.BeamRechargeBonus], m_config[m_config.EModBeamRechargeBonus], obj, -4095, 4095)) / 100
             + getExperienceModifiedValue(m_config[m_config.BeamRechargeRate], m_config[m_config.EModBeamRechargeRate], obj, 0, 16384))
        * obj.getBeamChargeRate();
    return i < 1 ? 1 : i;
}

/** Recharge beams. */
inline void
game::vcr::classic::PVCRAlgorithm::beamRecharge(Status& st)
{
    // ex VcrPlayerPHost::beamRecharge
    register const int mx = st.r.obj.getNumBeams();
    for (int i = 0; i < mx; ++i) {
        if (st.r.m_beamStatus[i] < 1000) {
            st.r.m_beamStatus[i] += randomRange(st.f.beam_recharge);
            visualizer().updateBeam(*this, st.f.side, i);
        }
    }
}

/** Find nearest-possible fighter. Returns fighter index [0,VCR_MAX_FTRS),
    or -1 if none. */
inline int
game::vcr::classic::PVCRAlgorithm::beamFindNearestFighter(const Status& st, const Status& opp) const
{
    // ex VcrPlayerPHost::beamFindNearestFighter
    // Only look for fighters if we expect to find some
    int fighter = -1;
    if (opp.r.m_activeFighters != 0) {
        int32_t mindist = st.f.BeamHitFighterRange+1;
        const int32_t my_x = st.r.m_objectX;
        const bool foaf = m_fireOnAttackFighters;

        int retreatingFighter = -1;
        bool hasAttackingFighter = false;

        // One-pass algorithm:
        // - if FireOnAttackFighters is set:
        //   . look for the closest attacking fighter that is in range
        //   . if no attacking fighter at all, look for the first returning one (need not be in range!)
        // - if FireOnAttackFighters is not set
        //   . look for the closest fighter in range
        for (int i = 0, limit = opp.f.MaxFightersLaunched; i < limit; ++i) {
            const int fs = opp.r.m_fighterStatus[i];
            if (fs != FighterIdle) {
                if (fs == FighterAttacks) {
                    hasAttackingFighter = true;
                }
                if (fs == FighterAttacks || !foaf) {
                    // "closest in range" rule
                    int32_t d = std::abs(my_x - opp.r.m_fighterX[i]);
                    if (d < mindist) {
                        mindist = d;
                        fighter = i;
                    }
                }
                if (fs != FighterAttacks && retreatingFighter < 0 && foaf) {
                    // "first returning" rule: just remember the first we saw
                    retreatingFighter = i;
                }
            }
        }
        if (foaf && !hasAttackingFighter) {
            fighter = retreatingFighter;
        }
    }

    return fighter;
}

/** Fire beams on /side/. \returns true iff battle ends. */
inline bool
game::vcr::classic::PVCRAlgorithm::beamFire(Status& st, Status& opp)
{
    // ex VcrPlayerPHost::beamFire
    const int beam_mx = st.r.obj.getNumBeams();
    for (int beam = 0; beam < beam_mx; ++beam) {
        /* Can we fire at a fighter? */
        if (st.r.m_beamStatus[beam] >= st.f.BeamHitFighterCharge) {
            /* PVCR tests `&& distance <= 100000', but that's not needed */
            bool missing = !randomRange100LT(st.f.beam_hit_odds);
            int fighter = beamFindNearestFighter(st, opp);
            if (fighter >= 0) {
                /* We fire at a fighter. */
                st.r.m_beamStatus[beam] = 0;
                visualizer().updateBeam(*this, st.f.side, beam);
                visualizer().fireBeam(*this, st.f.side, -1-beam, fighter, missing ? -1 : +1, st.f.beam_damage, st.f.beam_kill);
                if (!missing) {
                    visualizer().killFighter(*this, opp.f.side, fighter);
                    opp.r.m_fighterStatus[fighter] = FighterIdle;
                    opp.r.m_activeFighters--;
                }
                return false;
            }
        }

        /* Can we fire at the enemy? */
        if ((opp.r.m_activeFighters <= 0 || (m_capabilities & game::v3::structures::BeamCapability) != 0)
            && st.r.m_beamStatus[beam] >= st.f.BeamHitShipCharge
            && getDistance() <= st.f.BeamFiringRange)
        {
            bool missing = !randomRange100LT(st.f.beam_hit_odds);
            int kill = (st.f.beam_kill  * (st.r.m_beamStatus[beam]/10) / 100) * st.r.obj.getBeamKillRate();
            int dest = st.f.beam_damage * (st.r.m_beamStatus[beam]/10) / 100;

            st.r.m_beamStatus[beam] = 0;
            visualizer().updateBeam(*this, st.f.side, beam);

            if (!missing) {
                bool hitr = hit(opp, kill, dest, st.f.beam_damage == 0);
                visualizer().fireBeam(*this, st.f.side, -1-beam, -1, 1, dest, kill);
                if (hitr)
                    return true;
            } else {
                visualizer().fireBeam(*this, st.f.side, -1-beam, -1, -1, dest, kill);
            }
            return false;
        }
    }
    return false;
}

/** Compute torpedo hit odds. Documented formula. Used in initialisation. */
int
game::vcr::classic::PVCRAlgorithm::computeTorpHitOdds(const game::spec::TorpedoLauncher& torp, const Object& obj) const
{
    // ex VcrPlayerPHost::computeTorpHitOdds
    int i = ((getExperienceModifiedValue(m_config[m_config.TorpHitBonus], m_config[m_config.EModTorpHitBonus], obj, -4095, 4095) * (torp.getKillPower() + torp.getDamagePower())) / 100
             + getExperienceModifiedValue(m_config[m_config.TorpHitOdds], m_config[m_config.EModTorpHitOdds], obj, 0, 100));
    return i < 0 ? 0 : i;
}

/** Compute torpedo recharge rate. Documented formula. Used in
    initialisation. */
int
game::vcr::classic::PVCRAlgorithm::computeTubeRechargeRate(const game::spec::TorpedoLauncher& torp, const Object& obj) const
{
    // ex VcrPlayerPHost::computeTubeRechargeRate
    int i = (((getExperienceModifiedValue(m_config[m_config.TubeRechargeBonus], m_config[m_config.EModTubeRechargeBonus], obj, -4095, 4095) * (torp.getKillPower() + torp.getDamagePower())) / 100
              + getExperienceModifiedValue(m_config[m_config.TubeRechargeRate], m_config[m_config.EModTubeRechargeRate], obj, 0, 16384)))
        * obj.getTorpChargeRate();
    return i < 1 ? 1 : i;
}

/** Recharge torpedo launchers. */
inline void
game::vcr::classic::PVCRAlgorithm::torpsRecharge(Status& st)
{
    // ex VcrPlayerPHost::torpsRecharge
    register const int mx = st.r.obj.getNumLaunchers();
    for (int i = 0; i < mx; ++i) {
        if (st.r.m_launcherStatus[i] < 1000) {
            st.r.m_launcherStatus[i] += randomRange(st.f.torp_recharge);
            visualizer().updateLauncher(*this, st.f.side, i);
        }
    }
}

/** Fire torpedoes. */
inline bool
game::vcr::classic::PVCRAlgorithm::torpsFire(Status& st, Status& opp)
{
    // ex VcrPlayerPHost::torpsFire
    if (getDistance() > st.f.TorpFiringRange || st.r.obj.getNumTorpedoes() == 0)
        return false;

    for (int launcher = 0, limit = st.r.obj.getNumLaunchers(); launcher < limit; ++launcher)
        if (st.r.m_launcherStatus[launcher] >= 1000) {
            /* we're firing a torpedo */
            int rr = randomRange100();

            st.r.obj.addTorpedoes(-1);
            st.r.m_launcherStatus[launcher] = 0;
            visualizer().updateLauncher(*this, st.f.side, launcher);
            if (rr <= st.f.torp_hit_odds) {
                /* Scaling factor for torpedo effect. Tim scales with 2 for some reason. */
                int kill = st.f.torp_kill;
                int damage = st.f.torp_damage;

                /* we hit the enemy */
                bool hitr = hit(opp, kill, damage, damage == 0);
                st.m_statistic.handleTorpedoHit();
                visualizer().fireTorpedo(*this, st.f.side, rr, launcher);
                return hitr;
            } else {
                /* miss */
                visualizer().fireTorpedo(*this, st.f.side, -1-rr, launcher);
                return false;
            }
        }
    return false;
}

/*
 *  Movement
 */

/** Move objects towards each other. */
inline void
game::vcr::classic::PVCRAlgorithm::moveObjects()
{
    // ex VcrPlayerPHost::moveObjects
    int32_t remain = getDistance() - m_standoffDistance;
    if (remain <= 0)
        return;

    /* move objects. Ensure StandoffDistance is not violated */
    int32_t move_left = std::min(remain, int32_t(m_status[LeftSide].f.ShipMovementSpeed));
    m_status[LeftSide].r.m_objectX += move_left;
    remain -= move_left;

    if (!m_status[RightSide].r.obj.isPlanet()) {
        int32_t move_right = std::min(remain, int32_t(m_status[RightSide].f.ShipMovementSpeed));
        m_status[RightSide].r.m_objectX -= move_right;
    }
}

/** Check whether object still has offensive capabilities. */
inline bool
game::vcr::classic::PVCRAlgorithm::canStillFight(const Status& st, const Status& opp) const
{
    // ex VcrPlayerPHost::canStillFight
    // FIXME: null-pointer checks!
    const bool drcheck = !(m_capabilities & game::v3::structures::DeathRayCapability) || !opp.r.obj.isPlanet();
    return (st.r.obj.getNumBeams() > 0 && (drcheck || st.f.beam_damage))
        || (st.r.obj.getNumFighters() > 0 && st.r.obj.getNumBays() > 0)
        || (st.r.m_activeFighters > 0)
        || (st.r.obj.getNumTorpedoes() > 0 && (drcheck || st.f.torp_damage));
}


/*
 *  Activity detection
 */

/** Initialize inactivity detection. This is called at initialisation
    of the battle and resets the detector. */
void
game::vcr::classic::PVCRAlgorithm::initActivityDetector()
{
    // ex VcrPlayerPHost::initActivityDetector
    det_valid = false;
    det_timer = DET_MOVEMENT_TIMER;
}

/** Compare inactivity detector status for one side.
    \param a     DetectorStatus object
    \param side  Side to compare with
    \return true iff DetectorStatus represents current status */
inline bool
game::vcr::classic::PVCRAlgorithm::compareDetectorStatus(const DetectorStatus& a, const Status& st)
{
    // ex VcrPlayerPHost::compareDetStatus
    return a.fighters == st.r.m_activeFighters + st.r.obj.getNumFighters()
        && a.torps    == st.r.obj.getNumTorpedoes()
#ifdef PVCR_INTEGER
        && a.shield_scaled == st.r.shield_scaled
        && a.damage_scaled == st.r.damage_scaled2
        && a.crew_scaled   == st.r.crew_scaled2
#else
        && a.shield  == st.r.shield
        && a.damage  == st.r.damage
        && a.crew    == st.r.crew
#endif
        ;
}

/** Fill in inactivity detector status for one side.
    \param a     [out] DetectorStatus object
    \param side  Side to fill in */
void
game::vcr::classic::PVCRAlgorithm::setDetectorStatus(DetectorStatus& a, const Status& st)
{
    // ex VcrPlayerPHost::fillInDetStatus
    a.fighters = st.r.m_activeFighters + st.r.obj.getNumFighters();
    a.torps    = st.r.obj.getNumTorpedoes();
#ifdef PVCR_INTEGER
    a.shield_scaled = st.r.shield_scaled;
    a.damage_scaled = st.r.damage_scaled2;
    a.crew_scaled   = st.r.crew_scaled2;
#else
    a.shield  = st.r.shield;
    a.damage  = st.r.damage;
    a.crew    = st.r.crew;
#endif
}

/** Check whether there is combat activity. This feature was added in
    PHost 4.1a to avoid infinite loops on accidentally / maliciously
    modified combat configurations. We need not link it to particular
    host versions, though, because it just avoids infinite loops and
    does not change actual results.

    This detector works by taking a snapshot of the relevant
    parameters of a ship, and periodically checking whether these
    parameters changed. In case they don't, it assumes an infinite
    loop without any progress and ends the fight. The interval between
    checks is chosen to minimize the possibility of false positives.
    It has the known deficiency of yielding false positives when
    weapon hit and recharge rates are unattractively low (recharge
    rate 2 + hit rate 1 yields one hit on average after 100'000
    ticks).

    \return true if combat shall continue, false if it shall end */
bool
game::vcr::classic::PVCRAlgorithm::checkCombatActivity()
{
    // ex VcrPlayerPHost::checkCombatActivity
    /* re-check timer expired? If not, don't check. */
    if (det_timer > m_time)
        return true;

    /* still moving? If yes, there's progress. */
    if (getDistance() > m_standoffDistance) {
        det_timer = m_time + DET_MOVEMENT_TIMER;
        return true;
    }

    /* Movement has stopped. Has there been any progress since last check? */
    if (det_valid) {
        if (compareDetectorStatus(m_detectorStatus[LeftSide], m_status[LeftSide]) && compareDetectorStatus(m_detectorStatus[RightSide], m_status[RightSide])) {
            /* No progress */
            return false;
        }
    }

    /* Combat still runs. Compute re-check time. */
    setDetectorStatus(m_detectorStatus[LeftSide], m_status[LeftSide]);
    setDetectorStatus(m_detectorStatus[RightSide], m_status[RightSide]);
    det_valid = true;

    int32_t interval = DET_INACTIVITY_TIMER;
    for (int i = 0; i < 2; ++i) {
        if (m_detectorStatus[i].fighters != 0) {
            int32_t L = m_status[i].f.BayLaunchInterval + 100;  // 100 = fuzz factor for safety
            if (m_status[i].f.FighterMovementSpeed > 0)
                L += 2 * m_standoffDistance / m_status[i].f.FighterMovementSpeed;
            if (L > interval)
                interval = L;
        }
    }
    det_timer = m_time + interval;
    return true;
}


/** Verify one side of VCR. */
bool
game::vcr::classic::PVCRAlgorithm::checkSide(Object& obj) const
{
    // ex VcrPlayerPHost::checkVcrSide
    bool err = false;

    if (obj.getOwner() <= 0 || obj.getOwner() > 12/*FIXME*/) {
        obj.setOwner(12);
    }

    if (obj.getBeamType() != 0 && m_beams.get(obj.getBeamType()) == 0) {
        obj.setBeamType(0);
        obj.setNumBeams(0);
        err = true;
    }
    if (obj.getTorpedoType() != 0 && m_launchers.get(obj.getTorpedoType()) == 0) {
        obj.setTorpedoType(0);
        obj.setNumLaunchers(0);
        err = true;
    }

    // validate weapon counts
    if (obj.getNumBeams() > VCR_MAX_BEAMS) {
        obj.setNumBeams(VCR_MAX_BEAMS);
        err = true;
    }

    if (obj.getNumLaunchers() > VCR_MAX_TORPS) {
        obj.setNumLaunchers(VCR_MAX_TORPS);
        err = true;
    }

    if (obj.getNumBays() > VCR_MAX_BAYS) {
        obj.setNumBays(VCR_MAX_BAYS);
        err = true;
    }

    // ensure experience level is consistent with configuration
    if (obj.getExperienceLevel()) {
        if (!(m_capabilities & game::v3::structures::ExperienceCapability) || obj.getExperienceLevel() > m_config[m_config.NumExperienceLevels]()) {
            obj.setExperienceLevel(0);
            err = true;
        }
    }

    return err;
}
