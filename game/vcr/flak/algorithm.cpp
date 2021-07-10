/**
  *  \file game/vcr/flak/algorithm.cpp
  *  \brief Class game::vcr::flak::Algorithm
  *
  *  Changes to original version:
  *  - move all I/O and setup code into class Setup
  *  - store per-fleet attack lists
  *  - store torps/fighters in a vector, not a manually-implemented linked list;
  *    this means we need to iterate backwards (vector uses push-back whereas list used push-front).
  *  - implemented Object pool (PCC1 has this as an option); also used for allocating Ids for visualizer.
  *  - adjustments to stored data
  *  - interface cleaned up
  */

#include <cassert>
#include <algorithm>
#include <cmath>
#include "game/vcr/flak/algorithm.hpp"
#include "afl/base/countof.hpp"
#include "game/playerset.hpp"
#include "game/vcr/flak/environment.hpp"
#include "game/vcr/flak/setup.hpp"
#include "game/vcr/flak/visualizer.hpp"
#include "util/math.hpp"

#define FLAK_CHECKPOINT 0
#if FLAK_CHECKPOINT
# include <iostream>
#endif

using game::vcr::flak::Environment;

/*
 *  Structure Definitions
 */

struct game::vcr::flak::Algorithm::Ship {
    // ex FlakShip

    /** Running status for this ship, variable. */
    struct Status {
        double  shield;
        double  damage;
        double  crew;
        bool    isAlive;
        const Ship* lastHitBy;
        int     beamCharge[FLAK_MAX_BEAMS];
        int     torpedoCharge[FLAK_MAX_TORPS];
        int     bayCharge[FLAK_MAX_BAYS];
        int     torpedoLimit;
        int     launchCountdown;
        int     numFightersLaunched;
        int     zOffset;
        int     numFighters;
        int     numReceivedTorpedoes;
        int     numTorpedoes;

        // Statistics stuff
        Statistic stat;
    };
    Status status;

    /** Configuration for this ship, constant during fight. */
    struct Config {
        int32_t StandoffDistance;
        int     BayRechargeRate;
        int     BeamRechargeRate;
        int     BeamHitOdds;
        int     TubeRechargeRate;
        int     TorpHitOdds;
        // MaxFightersLaunched is in data
        int     ShieldKillScaling;
        int     ShieldDamageScaling;
        int     CrewKillScaling;
        int     HullDamageScaling;
        int     StrikesPerFighter;
        int     FighterMovementSpeed;
        int     FighterBeamKill;
        int     FighterBeamExplosive;
        int     BayLaunchInterval;
        int32_t TorpFiringRange;
        int     BeamHitFighterCharge;
        int     BeamHitShipCharge;
        int32_t BeamFiringRange;
        bool    FireOnAttackFighters;
        int     FighterKillOdds;
        int     FighterFiringRange;
        Config(const game::vcr::flak::Object& d, const Environment& env);
    };
    const Config config;

    /** Ship data: all data as supplied by the host. */
    struct Data {
        size_t shipIndex;         // ex status.number
        int numBeams;
        int numLaunchers;
        int numTorpedoes;
        int numBays;
        int numFighters;
        bool isPlanet;
        int torpedoType;
        int beamType;
        int initialShield;
        int initialDamage;
        int initialCrew;
        int mass;
        int player;
        int32_t rating;
        int maxFightersLaunched;
        int compensation;
        int id;
        String_t name;
        Data(size_t shipIndex, const game::vcr::flak::Object& d);
    };
    const Data data;

    /** Link to our fleet. */
    const Fleet& fleetLink;

    Ship(size_t shipIndex, const Fleet& fleetLink, const game::vcr::flak::Object& data, const Environment& env);

    void init();

    bool isAlive() const
        { return status.isAlive; }
    bool isPlanet() const
        { return data.isPlanet; }
    Position getPos() const;

    static Visualizer::Ship_t getShipNumber(const Ship* p)
        { return p != 0 ? p->data.shipIndex : Visualizer::NO_ENEMY; }
};

struct game::vcr::flak::Algorithm::Fleet {
    // ex FlakFleet

    /** Running status, variable. */
    struct Status {
        Ship*      enemy_ptr;
        bool       alive;
        Position   position;
    };

    struct Data {
        int player;
        size_t firstShipIndex;
        size_t numShips;
        int speed;
        int32_t x_init, y_init;
        std::vector<int16_t> attackList;
        Data(const Setup::Fleet& f, const Setup::AttackList_t& attList)
            : player(f.player),
              firstShipIndex(f.firstShipIndex),
              numShips(f.numShips),
              speed(f.speed),
              x_init(f.x), y_init(f.y),
              attackList(attList.begin() + 2*f.firstAttackListIndex,
                         attList.begin() + 2*(f.firstAttackListIndex + f.numAttackListEntries))
            { }
    };
    const Data data;

    /** Running status. */
    Status status;
    /** New position after movement. Only valid during position-recompute phase. */
    Position newPosition;

    Fleet(const Setup::Fleet& f, const Setup::AttackList_t& attList);

    /** Check whether this fleet is still alive. */
    bool isAlive() const
        { return status.alive; }
    void init();
};

struct game::vcr::flak::Algorithm::Object {
    // ex FlakObject
    ObjectKind kind : 8;                ///< Object kind.
    bool       canChangeEnemy;          ///< True if this object can change its enemy (applicable to fighters).
    Position   position;                ///< Current position.
    Ship*      enemy_ptr;               ///< Enemy ship (our target).
    Ship*      owner_ptr;               ///< Owner ship (who launched us).
    int        strikes;                 ///< Strikes left. For torps, nonzero if it hits, zero if it misses.
    int        kill, expl, death_flag;  ///< Weapon characteristics.
    int        speed;                   ///< Movement speed.
    size_t     visId;                   ///< Id for use by visualizer. ex vis_data.

    Object(size_t visId)
        : kind(), canChangeEnemy(), position(), enemy_ptr(), owner_ptr(),
          strikes(), kill(), expl(), death_flag(), speed(), visId(visId)
        { }
};


namespace {
    /*
     *  Formulas
     */

    int getExperienceConfiguration(const Environment& env, Environment::ExperienceOption index, int level, int player, int min, int max)
    {
        return std::max(min, std::min(max, env.getExperienceConfiguration(index, level, player)));
    }

    int computeBayRechargeRate(int numBays, int level, int pid, const Environment& env)
    {
        // ex flak.pas:P_BayRechargeRate
        int val = getExperienceConfiguration(env, Environment::BayRechargeRate, level, pid, 0, 16384) +
            numBays * getExperienceConfiguration(env, Environment::BayRechargeBonus, level, pid, -500, 500);
        return val < 1 ? 1 : val;
    }

    int computeBeamRechargeRate(int beamType, int level, int pid, const Environment& env)
    {
        // ex flak.pas:P_BeamRechargeRate (takes lower limit at 0, but that doesn't make a difference with our RNG)
        int smash = env.getBeamKillPower(beamType) + env.getBeamDamagePower(beamType);
        int val = getExperienceConfiguration(env, Environment::BeamRechargeRate, level, pid, 0, 16384)
            + smash * getExperienceConfiguration(env, Environment::BeamRechargeBonus, level, pid, -4095, 4095) / 100;
        return val < 1 ? 1 : val;
    }

    int computeBeamHitOdds(int beamType, int level, int pid, const Environment& env)
    {
        // ex flak.pas:P_BeamHitOdds
        int smash = env.getBeamKillPower(beamType) + env.getBeamDamagePower(beamType);
        int val = getExperienceConfiguration(env, Environment::BeamHitOdds, level, pid, 0, 100)
            + smash * getExperienceConfiguration(env, Environment::BeamHitBonus, level, pid, -4095, 4095) / 100;
        return val < 0 ? 0 : val;
    }

    int computeTubeRechargeRate(int torpedoType, int level, int pid, const Environment& env)
    {
        // ex flak.pas:P_TubeRechargeRate
        int smash = env.getTorpedoKillPower(torpedoType) + env.getTorpedoDamagePower(torpedoType);
        int val = getExperienceConfiguration(env, Environment::TubeRechargeRate, level, pid, 0, 16384)
            + smash * getExperienceConfiguration(env, Environment::TubeRechargeBonus, level, pid, -4095, 4095) / 100;
        return val < 1 ? 1 : val;
    }

    int computeTorpHitOdds(int torpedoType, int level, int pid, const Environment& env)
    {
        // ex flak.pas:P_TorpHitOdds
        int smash = env.getTorpedoKillPower(torpedoType) + env.getTorpedoDamagePower(torpedoType);
        int val = getExperienceConfiguration(env, Environment::TorpHitOdds, level, pid, 0, 100)
            + smash * getExperienceConfiguration(env, Environment::TorpHitBonus, level, pid, -4095, 4095) / 100;
        return val < 0 ? 0 : val;
    }

    /** Compute shield damage.
        \param expl,kill  weapon parameters
        \param mass       mass of unit being hit
        \param who        unit which fires the weapon */
    double computeShieldDamage(int expl, int kill, int mass, const game::vcr::flak::Algorithm::Ship& who, bool alternativeCombat)
    {
        // ex flak.pas:P_ShieldDamage
        double damage = (who.config.ShieldKillScaling * double(kill)
                         + who.config.ShieldDamageScaling * double(expl)) / (mass + 1);
        return damage > 10000
            ? 10000
            : alternativeCombat
            ? damage
            : int(damage + 1.5);
    }

    /** Compute hull damage.
        \param expl,kill  weapon parameters
        \param mass       mass of unit being hit
        \param who        unit which fires the weapon */
    double computeHullDamage(int expl, int kill, int mass, const game::vcr::flak::Algorithm::Ship& who, bool alternativeCombat)
    {
        // ex flak.pas:P_HullDamage
        if (alternativeCombat) {
            double d = double(expl) * who.config.HullDamageScaling / (mass+1);
            return d > 10000 ? 10000 : d;
        } else {
            double d = computeShieldDamage(expl, kill, mass, who, alternativeCombat) * who.config.HullDamageScaling / (mass+1);
            return d > 10000 ? 10000 : int(d + 1.5);
        }
    }

    /** Compute crew killed.
        \param expl,kill  weapon parameters
        \param death_flag zero if this is a death ray, nonzero otherwise
        \param mass       mass of unit being hit
        \param who        unit which fires the weapon */
    double computeCrewKilled(int kill, int mass, int death_flag, const game::vcr::flak::Algorithm::Ship& who, bool alternativeCombat)
    {
        // ex flak.pas:P_CrewKilled
        double d = double(kill) * who.config.CrewKillScaling / (mass+1);
        if (alternativeCombat) {
            return d;
        } else {
            long el = long(d + 0.5);
            if (el == 0 && death_flag == 0) {
                return 1.0;
            } else {
                return double(el);
            }
        }
    }

    /** Check whether /we/ can attack /they/. */
    bool canStillAttack(const game::vcr::flak::Algorithm::Ship& we, const game::vcr::flak::Algorithm::Ship& they, const Environment& env)
    {
        // ex flak.pas:CanStillAttack
        int torpc = we.data.numLaunchers;
        int beamc = we.data.numBeams;

        if (they.isPlanet()) {
            /* discount death rays against planet */
            if (torpc && env.getTorpedoDamagePower(we.data.torpedoType) == 0) {
                torpc = 0;
            }
            if (beamc && env.getBeamDamagePower(we.data.beamType) == 0
                && they.status.numFighters == 0
                && they.status.numFightersLaunched == 0)
            {
                beamc = 0;
            }
        }

        return beamc != 0
            || (torpc != 0 && we.status.numTorpedoes != 0)
            || (we.data.numBays != 0
                && (we.status.numFighters != 0
                    || we.status.numFightersLaunched != 0));
    }


    template<typename T>
    void copyList(afl::container::PtrVector<T>& out, const afl::container::PtrVector<T>& in)
    {
        for (size_t i = 0, n = in.size(); i < n; ++i) {
            out.pushBackNew(new T(*in[i]));
        }
    }


    void clearFlakLog()
    { }

    void addFlakLog(const char*)
    { }
}

/*
 *  game::vcr::flak::Algorithm::Ship
 */

game::vcr::flak::Algorithm::Ship::Config::Config(const game::vcr::flak::Object& d, const Environment& env)
    : StandoffDistance    (env.getConfiguration(Environment::StandoffDistance)),
      BayRechargeRate     (computeBayRechargeRate(d.getNumBays(), d.getExperienceLevel(), d.getOwner(), env)),
      BeamRechargeRate    (d.getNumBeams()     ? computeBeamRechargeRate(d.getBeamType(),        d.getExperienceLevel(), d.getOwner(), env) : 0),
      BeamHitOdds         (d.getNumBeams()     ? computeBeamHitOdds     (d.getBeamType(),        d.getExperienceLevel(), d.getOwner(), env) : 0),
      TubeRechargeRate    (d.getNumLaunchers() ? computeTubeRechargeRate(d.getTorpedoType(),     d.getExperienceLevel(), d.getOwner(), env) : 0),
      TorpHitOdds         (d.getNumLaunchers() ? computeTorpHitOdds     (d.getTorpedoType(),     d.getExperienceLevel(), d.getOwner(), env) : 0),
      ShieldKillScaling   (getExperienceConfiguration(env, Environment::ShieldKillScaling,       d.getExperienceLevel(), d.getOwner(), 0, 32767)),
      ShieldDamageScaling (getExperienceConfiguration(env, Environment::ShieldDamageScaling,     d.getExperienceLevel(), d.getOwner(), 0, 32767)),
      CrewKillScaling     (getExperienceConfiguration(env, Environment::CrewKillScaling,         d.getExperienceLevel(), d.getOwner(), 0, 32767)),
      HullDamageScaling   (getExperienceConfiguration(env, Environment::HullDamageScaling,       d.getExperienceLevel(), d.getOwner(), 0, 32767)),
      StrikesPerFighter   (getExperienceConfiguration(env, Environment::StrikesPerFighter,       d.getExperienceLevel(), d.getOwner(), 1, 100)),
      FighterMovementSpeed(getExperienceConfiguration(env, Environment::FighterMovementSpeed,    d.getExperienceLevel(), d.getOwner(), 1, 10000)),
      FighterBeamKill     (getExperienceConfiguration(env, Environment::FighterBeamKill,         d.getExperienceLevel(), d.getOwner(), 1, 1000)),
      FighterBeamExplosive(getExperienceConfiguration(env, Environment::FighterBeamExplosive,    d.getExperienceLevel(), d.getOwner(), 1, 1000)),
      BayLaunchInterval   (env.getConfiguration(Environment::BayLaunchInterval,    d.getOwner())),
      TorpFiringRange     (env.getConfiguration(Environment::TorpFiringRange,      d.getOwner())),
      BeamHitFighterCharge(getExperienceConfiguration(env, Environment::BeamHitFighterCharge,    d.getExperienceLevel(), d.getOwner(), 0, 1000)),
      BeamHitShipCharge   (env.getConfiguration(Environment::BeamHitShipCharge,    d.getOwner())),
      BeamFiringRange     (env.getConfiguration(Environment::BeamFiringRange,      d.getOwner())),
      FireOnAttackFighters(env.getConfiguration(Environment::FireOnAttackFighters)),
      FighterKillOdds     (env.getConfiguration(Environment::FighterKillOdds,      d.getOwner())),
      FighterFiringRange  (env.getConfiguration(Environment::FighterFiringRange,   d.getOwner()))
{ }

game::vcr::flak::Algorithm::Ship::Data::Data(size_t shipIndex, const game::vcr::flak::Object& d)
    : shipIndex(shipIndex), numBeams(d.getNumBeams()), numLaunchers(d.getNumLaunchers()), numTorpedoes(d.getNumTorpedoes()), numBays(d.getNumBays()),
      numFighters(d.getNumFighters()), isPlanet(d.isPlanet()), torpedoType(d.getTorpedoType()), beamType(d.getBeamType()),
      initialShield(d.getShield()), initialDamage(d.getDamage()), initialCrew(d.getCrew()), mass(d.getMass()),
      player(d.getOwner()), rating(d.getRating()), maxFightersLaunched(d.getMaxFightersLaunched()),
      compensation(d.getCompensation()), id(d.getId()), name(d.getName())
{ }

game::vcr::flak::Algorithm::Ship::Ship(size_t shipIndex, const Fleet& fleetLink, const game::vcr::flak::Object& data, const Environment& env)
    : status(), config(data, env), data(shipIndex, data), fleetLink(fleetLink)
{
    // ex FlakShip::FlakShip
    status.stat.init(data, 1);
    init();
}

void
game::vcr::flak::Algorithm::Ship::init()
{
    // ex FlakShip::init(int number), flak.pas:FlakShipInit
    status.shield              = data.initialShield;
    status.damage              = data.initialDamage;
    status.crew                = data.initialCrew;
    status.isAlive             = true;
    status.lastHitBy           = 0;
    status.torpedoLimit        = data.numLaunchers;
    status.launchCountdown     = 0;
    status.numFightersLaunched = 0;
    status.zOffset             = 0;   // will be computed elsewhere
    status.numFighters         = data.numFighters;
    status.numTorpedoes        = data.numTorpedoes;

    int st = data.initialShield == 100 ? 1000 : 0;
    for (size_t i = 0; i < countof(status.beamCharge); ++i) {
        status.beamCharge[i] = st;
    }
    for (size_t i = 0; i < countof(status.torpedoCharge); ++i) {
        status.torpedoCharge[i] = st;
    }
    for (size_t i = 0; i < countof(status.bayCharge); ++i) {
        status.bayCharge[i] = st;
    }
    status.numReceivedTorpedoes = 0;

    // Statistics (min_fighters_aboard, torps_hit) initialized in constructor
}

game::vcr::flak::Position
game::vcr::flak::Algorithm::Ship::getPos() const
{
    Position pos = fleetLink.status.position;
    pos.z += status.zOffset;
    return pos;
}


/*
 *  game::vcr::flak::Algorithm::Fleet
 */

game::vcr::flak::Algorithm::Fleet::Fleet(const Setup::Fleet& f, const Setup::AttackList_t& attList)
    : data(f, attList), status(), newPosition()
{
    // ex FlakFleet::FlakFleet
    init();
}

void
game::vcr::flak::Algorithm::Fleet::init()
{
    // ex FlakFleet::init, flak.pas:FlakFleetInit
    status.enemy_ptr = 0;
    status.alive = true;
    status.position.x = data.x_init;
    status.position.y = data.y_init;
    status.position.z = 0;
}



/*
 *  game::vcr::flak::Algorithm::Player
 */

struct game::vcr::flak::Algorithm::Player {
    // ex FlakPlayer
    const int16_t number;     ///< Player number.
    int16_t  num_live_ships;  ///< Number of living ships this player has.
    int32_t  sum_strength;    ///< Total strength, for compensation rating.
    afl::container::PtrVector<Object> stuff; ///< Active fighters and torps.
    bool     have_any_fighters; ///< True iff this player has (had) any fighters out.
    int      FighterKillOdds;

    Player(int number);
    ~Player();
    Player& operator=(const Player&);
    void init(const Environment& env);
};

game::vcr::flak::Algorithm::Player::Player(int number)
    : number(static_cast<int16_t>(number)), num_live_ships(0), sum_strength(0), stuff(), have_any_fighters(false)
{
    // ex FlakPlayer::FlakPlayer
}

game::vcr::flak::Algorithm::Player::~Player()
{
    // ex FlakPlayer::~FlakPlayer
}

game::vcr::flak::Algorithm::Player&
game::vcr::flak::Algorithm::Player::operator=(const Player& other)
{
    num_live_ships    = other.num_live_ships;
    sum_strength      = other.sum_strength;
    have_any_fighters = other.have_any_fighters;
    FighterKillOdds   = other.FighterKillOdds;
    stuff.clear();
    copyList(stuff, other.stuff);
    return *this;
}

void
game::vcr::flak::Algorithm::Player::init(const Environment& env)
{
    // ex FlakPlayer::init
    stuff.clear();
    num_live_ships = 0;
    sum_strength = 0;
    have_any_fighters = false;
    FighterKillOdds = env.getConfiguration(Environment::FighterKillOdds, number);
}


/*
 *  game::vcr::flak::Algorithm::StatusTokenImpl
 */

class game::vcr::flak::Algorithm::StatusTokenImpl : public StatusToken {
    // ex FlakStatusToken
    afl::container::PtrVector<Fleet::Status> m_fleets;
    afl::container::PtrVector<Ship::Status> m_ships;
    afl::container::PtrVector<Player> m_playerStatus;
    uint32_t m_seed;
    int32_t m_time;
    bool m_isTerminated;

    std::vector<size_t> m_unusedObjectIds;
    size_t m_objectId;

 public:
    StatusTokenImpl(const Algorithm& battle);
    ~StatusTokenImpl();
    void storeTo(Algorithm& battle) const;
};

game::vcr::flak::Algorithm::StatusTokenImpl::StatusTokenImpl(const Algorithm& battle)
    : m_fleets(), m_ships(),
      m_seed(battle.m_seed), m_time(battle.m_time), m_isTerminated(battle.m_isTerminated),
      m_objectId(battle.m_objectId)
{
    // ex FlakStatusToken::FlakStatusToken
    m_fleets.reserve(battle.m_fleets.size());
    for (size_t i = 0; i < battle.m_fleets.size(); ++i) {
        m_fleets.pushBackNew(new Fleet::Status(battle.m_fleets[i]->status));
    }
    m_ships.reserve(battle.m_ships.size());
    for (size_t i = 0; i < battle.m_ships.size(); ++i) {
        m_ships.pushBackNew(new Ship::Status(battle.m_ships[i]->status));
    }
    for (size_t i = 0; i < battle.m_playerStatus.size(); ++i) {
        m_playerStatus.pushBackNew(new Player(int(i+1)));
        *m_playerStatus.back() = *battle.m_playerStatus[i];
    }
    for (size_t i = 0; i < battle.m_unusedObjects.size(); ++i) {
        m_unusedObjectIds.push_back(battle.m_unusedObjects[i]->visId);
    }
}

game::vcr::flak::Algorithm::StatusTokenImpl::~StatusTokenImpl()
{ }

void
game::vcr::flak::Algorithm::StatusTokenImpl::storeTo(Algorithm& battle) const
{
    assert(battle.m_ships.size() == m_ships.size());
    assert(battle.m_fleets.size() == m_fleets.size());
    assert(battle.m_playerStatus.size() == m_playerStatus.size());

    for (std::size_t i = 0; i < battle.m_fleets.size(); ++i) {
        battle.m_fleets[i]->status = *m_fleets[i];
    }
    for (std::size_t i = 0; i < battle.m_ships.size(); ++i) {
        battle.m_ships[i]->status = *m_ships[i];
    }
    for (std::size_t i = 0; i < battle.m_playerStatus.size(); ++i) {
        *battle.m_playerStatus[i] = *m_playerStatus[i];
    }
    battle.m_seed = m_seed;
    battle.m_time = m_time;
    battle.m_isTerminated = m_isTerminated;

    battle.m_unusedObjects.clear();
    for (size_t i = 0; i < m_unusedObjectIds.size(); ++i) {
        battle.m_unusedObjects.pushBackNew(new Object(m_unusedObjectIds[i]));
    }
    battle.m_objectId = m_objectId;
}


/*
 *  game::vcr::flak::Algorithm
 */

game::vcr::flak::Algorithm::Algorithm(const Setup& b, const Environment& env)
    : m_playerIndex(),
      m_alternativeCombat(env.getConfiguration(Environment::AllowAlternativeCombat)),
      m_fireOnAttackFighters(env.getConfiguration(Environment::FireOnAttackFighters)),
      m_unusedObjects(), m_objectId(),
      m_seed(b.getSeed()), m_originalSeed(b.getSeed()), m_time(0), m_isTerminated(false)
{
    // ex FlakBattle::FlakBattle
    /* copy fleets */
    for (size_t i = 0; i < b.getNumFleets(); ++i) {
        m_fleets.pushBackNew(new Fleet(b.getFleetByIndex(i), b.getAttackList()));
    }

    /* copy ships */
    for (size_t i = 0; i < b.getNumShips(); ++i) {
        // Simple + stupid
        const Fleet* f = 0;
        for (size_t ff = 0; ff < getNumFleets(); ++ff) {
            const Fleet& me = *m_fleets[ff];
            if (me.data.firstShipIndex <= i && i < me.data.firstShipIndex + me.data.numShips) {
                f = &me;
                break;
            }
        }
        assert(f);
        m_ships.pushBackNew(new Ship(i, *f, b.getShipByIndex(i), env));
    }
}

game::vcr::flak::Algorithm::~Algorithm()
{ }

// Initialize player.
void
game::vcr::flak::Algorithm::init(const Environment& env, Visualizer& vis)
{
    // ex FlakBattle::init, flak.pas:FlakBattleInit

    m_time          = 0;
    m_seed          = m_originalSeed;
    m_isTerminated = false;

    clearFlakLog();

    m_playerStatus.clear();
    for (size_t i = 0; i < m_ships.size(); ++i) {
        m_ships[i]->init();
    }
    for (size_t i = 0; i < m_fleets.size(); ++i) {
        m_fleets[i]->init();
    }

    for (size_t i = 0; i < m_fleets.size(); ++i) {
        // FIXME: maybe merge this with Fleet::init() ?
        Fleet& fl = *m_fleets[i];
        int cur_z = -int(fl.data.numShips-1) * 50;
        const size_t last = fl.data.firstShipIndex + fl.data.numShips;
        for (size_t s = fl.data.firstShipIndex; s < last; ++s) {
            m_ships[s]->status.zOffset = cur_z;
            cur_z += 100;

            size_t index = fl.data.player-1;
            while (m_playerStatus.size() <= index) {
                m_playerStatus.pushBackNew(new Player(int(m_playerStatus.size()+1)));
                m_playerStatus.back()->init(env);
            }
            m_playerStatus[fl.data.player-1]->num_live_ships++;
            m_playerStatus[fl.data.player-1]->sum_strength += m_ships[s]->data.compensation;
        }
        fl.status.alive = true;
    }

    m_playerIndex.clear();
    for (size_t i = 0; i < m_playerStatus.size(); ++i) {
        if (m_playerStatus[i]->num_live_ships != 0) {
            m_playerIndex.push_back(m_playerStatus[i]);
        }
    }

    renderAll(vis);
    for (size_t i = 0; i < m_fleets.size(); ++i) {
        chooseEnemy(*m_fleets[i], env, vis, i);
    }
}

// Play one cycle.
bool
game::vcr::flak::Algorithm::playCycle(const Environment& env, Visualizer& vis)
{
    // ex FlakBattle::playOneTick, flak.pas:FlakPlayOneTick
    // @change The return value differs from the PCC1/PCC2 implementation.
    if (m_isTerminated) {
        return false;
    }

    // recharge
    for (size_t i = 0; i < m_ships.size(); ++i) {
        rechargeShip(*m_ships[i]);
    }

    // choose enemy
    if (m_time != 0 && m_time % FLAK_CHOOSE_ENEMY_TIME == 0) {
        for (size_t i = 0; i < m_fleets.size(); ++i) {
            chooseEnemy(*m_fleets[i], env, vis, i);
        }
    }

    // launch fighters
    for (size_t i = 0; i < m_fleets.size(); ++i) {
        launchFighters(*m_fleets[i], vis);
    }

    // fire torps
    for (size_t i = 0; i < m_fleets.size(); ++i) {
        fireTorps(*m_fleets[i], env, vis);
    }

    // fire beams. We choose a random fleet to fire first to achieve
    // random distribution of hits.
    size_t fleet_off = random(int(m_fleets.size()));
    for (size_t i = fleet_off; i < m_fleets.size(); ++i) {
        fireBeams(*m_fleets[i], env, vis);
    }
    for (size_t i = 0; i < fleet_off; ++i) {
        fireBeams(*m_fleets[i], env, vis);
    }

    // fighters fire. We choose a random player to fire first to
    // achieve random distribution of hits
    // Distribution should be uniform among players taking part,
    // not among all players
    const size_t numPlayers = m_playerIndex.size();
    const size_t pivot = random(int(numPlayers));
    for (size_t i = pivot; i < numPlayers; ++i) {
        fightersFire(*m_playerIndex[i], vis);
    }
    for (size_t i = 0; i < pivot; ++i) {
        fightersFire(*m_playerIndex[i], vis);
    }

    // fighter intercept
    for (size_t i = 0; i+1 < numPlayers; ++i) {
        if (m_playerIndex[i]->have_any_fighters) {
            for (size_t j = i+1; j < numPlayers; ++j) {
                if (m_playerIndex[j]->have_any_fighters) {
                    // fighterIntercept prefers having a "young" fighter of
                    // the first player firing at an "old" one of the second
                    // one. Since no way is known to fix that, we randomly
                    // swap the players' roles.
                    if (random(2) == 0) {
                        fighterIntercept(*m_playerIndex[i], *m_playerIndex[j], vis);
                    } else {
                        fighterIntercept(*m_playerIndex[j], *m_playerIndex[i], vis);
                    }
                }
            }
        }
    }

    // move stuff
    for (size_t i = pivot; i < numPlayers; ++i) {
        moveStuff(*m_playerIndex[i], vis);
    }
    for (size_t i = 0; i < pivot; ++i) {
        moveStuff(*m_playerIndex[i], vis);
    }

    // gc
    for (size_t i = 0; i < m_fleets.size(); ++i) {
        doFleetGC(*m_fleets[i], env, vis, i);
    }

    // playergc
    for (size_t i = 0; i < numPlayers; ++i) {
        doPlayerGC(*m_playerIndex[i]);
    }

    // move units
    for (size_t i = 0; i < m_fleets.size(); ++i) {
        computeNewPosition(*m_fleets[i], env, vis, i);
    }
    for (size_t i = 0; i < m_fleets.size(); ++i) {
        Fleet& fleet = *m_fleets[i];
        if (fleet.status.position != fleet.newPosition) {
            fleet.status.position = fleet.newPosition;
            vis.moveFleet(i, fleet.status.position.x, fleet.status.position.y);
            for (size_t j = 0; j < fleet.data.numShips; ++j) {
                const size_t shipIndex = fleet.data.firstShipIndex + j;
                const Ship& sh = *m_ships[shipIndex];
                if (sh.isAlive()) {
                    vis.moveShip(shipIndex, sh.getPos());
                }
            }
        }
    }

#if FLAK_CHECKPOINT
    printCheckpoint(std::cout);
#endif

    ++m_time;
    vis.updateTime(m_time);

    // end check
    m_isTerminated = endCheck();
    return true;
}

// Create status token
game::vcr::flak::Algorithm::StatusToken*
game::vcr::flak::Algorithm::createStatusToken() const
{
    return new StatusTokenImpl(*this);
}

// Get current time.
int32_t
game::vcr::flak::Algorithm::getTime() const
{
    return m_time;
}

/*
 *  Ship Access
 */

size_t
game::vcr::flak::Algorithm::getNumShips() const
{
    return m_ships.size();
}

bool
game::vcr::flak::Algorithm::isPlanet(size_t shipIndex) const
{
    return shipIndex < m_ships.size() && m_ships[shipIndex]->data.isPlanet;
}

int
game::vcr::flak::Algorithm::getShipId(size_t shipIndex) const
{
    return shipIndex < m_ships.size() ? m_ships[shipIndex]->data.id : 0;
}

int
game::vcr::flak::Algorithm::getBeamStatus(size_t shipIndex, int id) const
{
    return ((shipIndex < m_ships.size() && id < FLAK_MAX_BEAMS)
            ? m_ships[shipIndex]->status.beamCharge[id]
            : 0);
}

int
game::vcr::flak::Algorithm::getLauncherStatus(size_t shipIndex, int id) const
{
    return ((shipIndex < m_ships.size() && id < FLAK_MAX_TORPS)
            ? m_ships[shipIndex]->status.torpedoCharge[id]
            : 0);
}

int
game::vcr::flak::Algorithm::getNumTorpedoes(size_t shipIndex) const
{
    return shipIndex < m_ships.size() ? m_ships[shipIndex]->status.numTorpedoes : 0;
}

int
game::vcr::flak::Algorithm::getNumFighters(size_t shipIndex) const
{
    return shipIndex < m_ships.size() ? m_ships[shipIndex]->status.numFighters : 0;
}

int
game::vcr::flak::Algorithm::getNumFightersLaunched(size_t shipIndex) const
{
    return shipIndex < m_ships.size() ? m_ships[shipIndex]->status.numFightersLaunched : 0;
}

int
game::vcr::flak::Algorithm::getFighterLaunchCountdown(size_t shipIndex) const
{
    return shipIndex < m_ships.size() ? m_ships[shipIndex]->status.launchCountdown : 0;
}

int
game::vcr::flak::Algorithm::getShield(size_t shipIndex) const
{
    return shipIndex < m_ships.size() ? int(m_ships[shipIndex]->status.shield) : 0;
}

int
game::vcr::flak::Algorithm::getDamage(size_t shipIndex) const
{
    return shipIndex < m_ships.size() ? int(m_ships[shipIndex]->status.damage) : 0;
}

int
game::vcr::flak::Algorithm::getCrew(size_t shipIndex) const
{
    return shipIndex < m_ships.size() ? int(m_ships[shipIndex]->status.crew) : 0;
}

game::vcr::Statistic
game::vcr::flak::Algorithm::getStatistic(size_t shipIndex) const
{
    return shipIndex < m_ships.size() ? m_ships[shipIndex]->status.stat : Statistic();
}


/*
 *  Fleet Access
 */

size_t
game::vcr::flak::Algorithm::getNumFleets() const
{
    return m_fleets.size();
}

game::vcr::flak::Position
game::vcr::flak::Algorithm::getFleetPosition(size_t number) const
{
    return (number < m_fleets.size()
            ? m_fleets[number]->status.position
            : Position());
}

bool
game::vcr::flak::Algorithm::isFleetAlive(size_t number) const
{
    return number < m_fleets.size() && m_fleets[number]->status.alive;
}


/*
 *  Result Access
 */

bool
game::vcr::flak::Algorithm::findCaptor(size_t shipIndex, util::RandomNumberGenerator& rng, size_t& captorIndex) const
{
    // ex FlakBattle::findCaptor
    if (shipIndex >= m_ships.size()) {
        return false;
    }
    const Ship& victim = *m_ships[shipIndex];

    /* captor still alive? */
    if (victim.status.lastHitBy == 0) {
        return false;
    }
    if (victim.status.lastHitBy->isAlive()) {
        for (size_t i = 0; i < getNumShips(); ++i) {
            if (victim.status.lastHitBy == m_ships[i]) {
                captorIndex = i;
                return true;
            }
        }
    }

    /* count all ships */
    int counts[FLAK_NUM_OWNERS+1];
    int total = 0;
    for (size_t i = 0; i < countof(counts); ++i) {
        counts[i] = 0;
    }
    for (size_t i = 0; i < getNumShips(); ++i) {
        const Ship& sh = *m_ships[i];
        if (sh.isAlive()) {
            ++counts[sh.data.player], ++total;
        }
    }

    /* captor race still alive? */
    int player = victim.status.lastHitBy->data.player;
    if (counts[player]) {
        int pick = rng(uint16_t(counts[player]));
        for (size_t i = 0; i < getNumShips(); ++i) {
            const Ship& sh = *m_ships[i];
            if (sh.isAlive() && sh.data.player == player) {
                if (--pick < 0) {
                    captorIndex = i;
                    return true;
                }
            }
        }
    }

    /* owner race still alive? */
    player = victim.data.player;
    if (counts[player]) {
        int pick = rng(uint16_t(counts[player]));
        for (size_t i = 0; i < getNumShips(); ++i) {
            const Ship& sh = *m_ships[i];
            if (sh.isAlive() && sh.data.player == player) {
                if (--pick < 0) {
                    captorIndex = i;
                    return true;
                }
            }
        }
    }

    /* anyone else alive? */
    if (total) {
        int pick = rng(uint16_t(total));
        for (size_t i = 0; i < getNumShips(); ++i) {
            const Ship& sh = *m_ships[i];
            if (sh.isAlive()) {
                if (--pick < 0) {
                    captorIndex = i;
                    return true;
                }
            }
        }
    }

    /* nobody hearing me? */
    return false;
}

void
game::vcr::flak::Algorithm::copyResult(size_t shipIndex, game::vcr::flak::Object& out) const
{
    // ex flak/flak-db.cc:copyAfter
    if (shipIndex < m_ships.size()) {
        const Ship& in = *m_ships[shipIndex];

        out.setShield(int(in.status.shield + 0.5));
        out.setDamage(int(in.status.damage + 0.5));
        out.setCrew(int(in.status.crew + 0.5));
        out.setNumTorpedoes(in.status.numTorpedoes);
        out.setNumFighters(in.status.numFighters);
    }
}

void
game::vcr::flak::Algorithm::setEndingStatus(Setup& battle, const Environment& env, util::RandomNumberGenerator& rng) const
{
    for (size_t i = 0, n = getNumShips(); i < n; ++i) {
        battle.getShipByIndex(i).setEndingStatus(findEndingStatus(i, env, rng));
    }
}

int
game::vcr::flak::Algorithm::findEndingStatus(size_t shipIndex, const Environment& env, util::RandomNumberGenerator& rng) const
{
    if (shipIndex < m_ships.size()) {
        const Ship& in = *m_ships[shipIndex];
        if (in.isAlive()) {
            // Survived
            return 0;
        } else {
            // Captured or died
            size_t captorIndex;
            if (findCaptor(shipIndex, rng, captorIndex)) {
                // Captor exists
                int captorPlayer = m_ships[captorIndex]->data.player;
                int limit = (env.getPlayerRaceNumber(in.data.player) == 2 && env.getPlayerRaceNumber(captorPlayer) == 2) ? 150 : 99;
                if (in.isPlanet() || (in.status.crew < 0.5 && util::roundToInt(in.status.damage) <= limit)) {
                    // Planet or captured ship
                    return captorPlayer;
                } else {
                    // Destroyed ship
                    return -1;
                }
            } else {
                // No captor
                return -1;
            }
        }
    } else {
        // Out-of-range
        return -1;
    }
}

/*
 *  Random number generator
 */

inline int
game::vcr::flak::Algorithm::random(int max)
{
    // ex FlakBattle::random, flak.pas:FlakRandom
    m_seed = 0x8088405U * m_seed + 1;
    return ((m_seed >> 16) * max) >> 16;
}


/*
 *  Object operations
 */

game::vcr::flak::Algorithm::Object&
game::vcr::flak::Algorithm::makeObject(Player& pl, ObjectKind kind)
{
    // FlakPlayer::addObject (sort-of)
    // Fetch object
    Object* obj;
    if (m_unusedObjects.empty()) {
        obj = new Object(++m_objectId);
    } else {
        obj = m_unusedObjects.extractLast();
    }

    // Add to player
    pl.stuff.pushBackNew(obj);

    // Initialize
    obj->kind = kind;
    return *obj;
}

void
game::vcr::flak::Algorithm::releaseObject(Object* obj)
{
    m_unusedObjects.pushBackNew(obj);
}

int32_t
game::vcr::flak::Algorithm::moveObjectTowards(Object& obj, Position toPos)
{
    // ex FlakObject::moveTowards, flak.pas:FlakObjectMoveTowards
    // FIXME: PCC1 performs optimisation here if position.y=to_pos.y (common case for 1:1 fight)
    const double dist = obj.position.distanceTo(toPos);
    const double newDist = dist - obj.speed;
    if (newDist <= 0) {
        obj.position = toPos;
        return 0;
    } else {
        obj.position.x = toPos.x + util::roundToInt(double(obj.position.x - toPos.x) * newDist / dist);
        obj.position.y = toPos.y + util::roundToInt(double(obj.position.y - toPos.y) * newDist / dist);
        obj.position.z = toPos.z + util::roundToInt(double(obj.position.z - toPos.z) * newDist / dist);
        return int32_t(newDist);
    }
}


/*
 *  Ship Operations
 */

/** Compute torpedo launch limit for attacker.
    \param attacker    the ship whose limit we're thinking about. The limit will be updated in-place
    \param enemy       desired enemy
    \param num_torpers total number of active torpers in attacking fleet
    \param env         Environment*/
void
game::vcr::flak::Algorithm::computeTorpLimit(Ship& attacker, const Ship& enemy, int num_torpers, const Environment& env) const
{
    // ex flak.pas:ComputeTorpLimit
    attacker.status.torpedoLimit = attacker.data.numLaunchers;

    /* don't bother for small ships or pathological case */
    if (attacker.data.numLaunchers <= 2 || attacker.config.TorpHitOdds <= 0) {
        return;
    }

    int expl = env.getTorpedoDamagePower(attacker.data.torpedoType);
    int kill = env.getTorpedoKillPower(attacker.data.torpedoType);
    if (!m_alternativeCombat) {
        expl *= 2, kill *= 2;
    }

    double cd = computeCrewKilled(kill, enemy.data.mass, expl /* death flag */, enemy, m_alternativeCombat);
    int torps_reqd;
    if (expl == 0) {
        /* death ray */
        if (enemy.isPlanet())             // should not happen
            return;
        // 'crew/cd' is number of hits required to kill crew
        // 'crew/cd * 100/TorpHitOdds' is average number of torps to kill crew
        // add safety factor
        torps_reqd = int(1 + (enemy.status.crew / cd) * FLAK_TORP_LIMIT_FACTOR / attacker.config.TorpHitOdds);
    } else {
        /* normal weapon */
        double hd = computeHullDamage(expl, kill, enemy.data.mass, enemy, m_alternativeCombat);
        double sd = computeShieldDamage(expl, kill, enemy.data.mass, enemy, m_alternativeCombat);

        int limit = (env.getPlayerRaceNumber(enemy.data.player) == 2) ? 151 : 100;
        double v1 = (limit - enemy.status.damage) / (hd + 0.01);
        if (!enemy.isPlanet()) {
            register double v2 = enemy.status.crew / (cd + 0.01);
            if (v1 > v2) {
                v1 = v2;
            }
        }
        torps_reqd = int(1 + (enemy.status.shield / sd + v1) * FLAK_TORP_LIMIT_FACTOR / attacker.config.TorpHitOdds);
    }

    if (num_torpers) {
        torps_reqd = (torps_reqd + num_torpers-1) / num_torpers;
    }

    if (torps_reqd < attacker.data.numLaunchers) {
        attacker.status.torpedoLimit = torps_reqd;
    }
}

/** Inflict damage to a unit.
    \param sh           target unit
    \param firing_ship  the ship which is responsible for hitting this ship
    \param expl,kill    weapon parameters
    \param death_flag   zero if it is a death ray */
void
game::vcr::flak::Algorithm::hitShipWith(Ship& sh, const Ship& firing_ship, int expl, int kill, int death_flag) const
{
    // ex FlakBattle::hitShipWith, flak.pas:HitShipWith
    const int att_count = m_playerStatus[firing_ship.data.player-1]->num_live_ships;
    const int opp_count = m_playerStatus[sh.data.player-1]->num_live_ships;
    const int32_t att_strength = m_playerStatus[firing_ship.data.player-1]->sum_strength;
    const int32_t opp_strength = m_playerStatus[sh.data.player-1]->sum_strength;

    /* compute effective mass */
    int eff_mass;
    if (att_count < opp_count && att_strength < opp_strength) {
        if ((att_strength + FLAK_COMPENSATION_DIVISOR) * FLAK_COMPENSATION_LIMIT < (opp_strength + FLAK_COMPENSATION_DIVISOR)) {
            eff_mass = sh.data.mass / FLAK_COMPENSATION_LIMIT;
        } else {
            eff_mass = sh.data.mass * (att_strength+FLAK_COMPENSATION_DIVISOR) / (opp_strength+FLAK_COMPENSATION_DIVISOR);
        }
    } else {
        eff_mass = sh.data.mass;
    }

    /* normal PHost damage processing */
    double damage_rate = 1.0;
    if (kill <= 0) {
        kill = 1;
    }
    if (expl <= 0) {
        expl = 1;
    }
    if (death_flag == 0) {
        goto death_ray;
    }

    if (sh.status.shield > 0) {
        double damage = computeShieldDamage(expl, kill, eff_mass, sh, m_alternativeCombat);
        if (sh.status.shield <= damage) {
            /* shield completely down */
            damage_rate = (damage - sh.status.shield) / damage;
            sh.status.shield = 0;
        } else {
            /* Shields still hold */
            damage_rate = 0.0;
            sh.status.shield -= damage;
        }
    }

    if (damage_rate <= 0) {
        goto vis;
    }

    sh.status.damage += computeHullDamage(expl, kill, eff_mass, sh, m_alternativeCombat) * damage_rate;
    if (sh.status.damage > 9999) {
        sh.status.damage = 9999;
    }

 death_ray:
    if (!sh.isPlanet()) {
        sh.status.crew -= computeCrewKilled(kill, eff_mass, death_flag, sh, m_alternativeCombat) * damage_rate;
        if (sh.status.crew < 0.5) {
            sh.status.crew = 0;
        }
    }

 vis:
    if (sh.isAlive()) {
        sh.status.lastHitBy = &firing_ship;
    }
}

void
game::vcr::flak::Algorithm::rechargeShip(Ship& ship)
{
    // ex FlakShip::recharge, FlakShipRecharge
    if (ship.isAlive()) {
        if (ship.status.launchCountdown > 0) {
            --ship.status.launchCountdown;
        }
        if (ship.status.numReceivedTorpedoes > 0) {
            ship.status.numReceivedTorpedoes--;
            ship.status.numTorpedoes++;
        }
        for (int i = 0; i < ship.data.numBays; ++i) {
            if (ship.status.bayCharge[i] < 1000) {
                ship.status.bayCharge[i] += random(ship.config.BayRechargeRate);
            }
        }
        for (int i = 0; i < ship.data.numBeams; ++i) {
            if (ship.status.beamCharge[i] < 1000) {
                ship.status.beamCharge[i] += random(ship.config.BeamRechargeRate);
            }
        }
        if (ship.status.numTorpedoes != 0) {
            for (int i = 0; i < ship.data.numLaunchers; ++i) {
                if (ship.status.torpedoCharge[i] < 1000) {
                    ship.status.torpedoCharge[i] += random(ship.config.TubeRechargeRate);
                }
            }
        }
    }
}

/*
 *  Player operations
 */

/** Garbage collection. Deletes all objects marked for deletion.
    \param p Player */
void
game::vcr::flak::Algorithm::doPlayerGC(Player& p)
{
    // ex FlakPlayer::doPlayerGC, FlakPlayerGC
    // FIXME: Pascal version has an optimisation with an additional 'have_any_died' flag
    size_t out = 0;
    for (size_t i = 0, n = p.stuff.size(); i < n; ++i) {
        if (p.stuff[i]->kind != oDeleteMe) {
            p.stuff.swapElements(i, out);
            ++out;
        }
    }
    while (p.stuff.size() > out) {
        releaseObject(p.stuff.extractLast());
    }
}


/*
 *  Combat Phases
 */

/* Pick a new enemy for a fleet. Updates the fleet in-place. */
void
game::vcr::flak::Algorithm::chooseEnemy(Fleet& fleet, const Environment& env, Visualizer& vis, size_t fleetNr)
{
    // ex FlakBattle::chooseEnemy, flak.pas:ChooseEnemy
    if (!fleet.isAlive()) {
        return;
    }
    int32_t best_diff = 0x7FFFFFFF;
    Ship*   best_choice = 0;
    for (size_t index = 0; index < fleet.data.attackList.size(); index += 2) {
        /* bail out early if we know we cannot attack it */
        const int their_ship_nr = fleet.data.attackList[index];
        Ship& their_ship = *m_ships[their_ship_nr];
        if (their_ship.data.player == fleet.data.player) {
            continue;
        }
        if (!their_ship.isAlive()) {
            continue;
        }

        /* figure out attack ratings */
        int32_t attack_rating    = 0;
        int32_t eff_rating_bonus = fleet.data.attackList[index + 1];

        for (size_t n = 0; n < fleet.data.numShips; ++n) {
            const Ship& we = *m_ships[n + fleet.data.firstShipIndex];
            if (we.isAlive() && (canStillAttack(we, their_ship, env) || canStillAttack(their_ship, we, env))) {
                attack_rating += we.data.rating;
            }
        }

        /* no attack rating means we cannot attack it at all */
        if (attack_rating == 0) {
            continue;
        }

        int32_t their_rating = their_ship.data.rating;
        int32_t divisor, diff;

        if (attack_rating < their_rating) {
            /* we're smaller */
            diff = their_rating - attack_rating + FLAK_DIFF_OFFSET;
            divisor = FLAK_DIVISOR_IF_SMALLER;
        } else {
            /* we're bigger */
            diff = attack_rating - their_rating + FLAK_DIFF_OFFSET;
            divisor = FLAK_DIVISOR_IF_BIGGER;
        }

        /* bonuses */
        divisor += eff_rating_bonus;
        if (&their_ship == fleet.status.enemy_ptr) {
            divisor += FLAK_DIVISOR_SAME_ENEMY_BONUS;
        }
        if (their_ship.status.damage > 0) {
            divisor += int(their_ship.status.damage);
        }
        divisor += (100 - int(their_ship.status.shield)) / 5;

        int32_t multiplier = int32_t(fleet.data.speed <= 0
                                     ? fleet.status.position.distanceTo(their_ship.fleetLink.status.position) / 100
                                     : fleet.status.position.distanceTo(their_ship.fleetLink.status.position) / fleet.data.speed);
        if (multiplier < FLAK_MULTIPLIER_MIN) {
            multiplier = FLAK_MULTIPLIER_MIN;
        }

        diff = diff * multiplier / divisor;
        if (diff < best_diff) {
            best_diff = diff;
            best_choice = &their_ship;
        }
    }

    if (best_choice != fleet.status.enemy_ptr) {
        vis.setEnemy(fleetNr, Ship::getShipNumber(best_choice));
        if (best_choice != 0 && fleet.status.enemy_ptr != 0) {
            addFlakLog("Target change in flight");
        }
    }

    fleet.status.enemy_ptr = best_choice;

    if (best_choice) {
        /* re-compute torp limit */
        int num_torpers = 0;
        for (size_t i = 0; i < fleet.data.numShips; ++i) {
            const Ship& sh = *m_ships[i+fleet.data.firstShipIndex];
            if (sh.isAlive() && sh.data.numLaunchers && sh.status.numTorpedoes >= 10) {
                ++num_torpers;
            }
        }
        for (size_t i = 0; i < fleet.data.numShips; ++i) {
            computeTorpLimit(*m_ships[i+fleet.data.firstShipIndex], *best_choice, num_torpers, env);
        }
    }
}

/* Launch fighters for given fleet. */
void
game::vcr::flak::Algorithm::launchFighters(const Fleet& fleet, Visualizer& vis)
{
    // ex FlakBattle::launchFighters, flask.pas:LaunchFighters
    if (!fleet.isAlive() || !fleet.status.enemy_ptr || !fleet.status.enemy_ptr->isAlive()) {
        return;
    }

    const size_t last = fleet.data.firstShipIndex + fleet.data.numShips;
    for (size_t i = fleet.data.firstShipIndex; i < last; ++i) {
        Ship& sh = *m_ships[i];
        if (sh.isAlive() && sh.data.numBays && sh.status.numFighters
            && sh.status.launchCountdown==0 && sh.status.numFightersLaunched < sh.data.maxFightersLaunched)
        {
            /* we can launch a fighter. Find a bay. */
            for (int bay = 0; bay < sh.data.numBays; ++bay) {
                if (sh.status.bayCharge[bay] < 1000) {
                    continue;
                }

                Object& p = makeObject(*m_playerStatus[sh.data.player-1], oFighter);

                p.canChangeEnemy   = true;
                p.position         = sh.getPos();
                p.enemy_ptr        = fleet.status.enemy_ptr;
                p.owner_ptr        = &sh;
                p.strikes          = sh.config.StrikesPerFighter;
                p.kill             = sh.config.FighterBeamKill;
                p.expl             = sh.config.FighterBeamExplosive;
                p.death_flag       = 1;
                p.speed            = sh.config.FighterMovementSpeed;

                sh.status.bayCharge[bay] = 0;
                sh.status.numFighters--;
                sh.status.numFightersLaunched++;
                sh.status.launchCountdown = sh.config.BayLaunchInterval;
                sh.status.stat.handleFightersAboard(sh.status.numFighters);

                m_playerStatus[sh.data.player-1]->have_any_fighters = true;
                vis.createFighter(p.visId, p.position, sh.data.player, Ship::getShipNumber(p.enemy_ptr));
                goto next_ship;
            }
        }
     next_ship:;
    }
}

/** Fire torps from a fleet. */
void
game::vcr::flak::Algorithm::fireTorps(const Fleet& fleet, const Environment& env, Visualizer& vis)
{
    // ex FlakBattle::fireTorps, flak.pas:FireTorps
    if (!fleet.isAlive() || !fleet.status.enemy_ptr || !fleet.status.enemy_ptr->isAlive()) {
        return;
    }

    const Position& ene_pos = fleet.status.enemy_ptr->fleetLink.status.position;
    const Position& my_pos  = fleet.status.position;

    const size_t last = fleet.data.numShips + fleet.data.firstShipIndex;
    for (size_t i = fleet.data.firstShipIndex; i < last; ++i) {
        Ship& sh = *m_ships[i];
        if (!sh.isAlive() || !sh.status.numTorpedoes) {
            continue;           // torp cannot fire
        }
        if (!my_pos.isDistanceLERadius(ene_pos, sh.config.TorpFiringRange)) {
            continue;
        }

        for (int tl = 0; tl < sh.status.torpedoLimit; ++tl) {
            if (sh.status.torpedoCharge[tl] < 1000) {
                continue;
            }

            Object& p = makeObject(*m_playerStatus[sh.data.player-1], oTorpedo);

            p.canChangeEnemy   = false;
            p.position         = sh.getPos();
            p.enemy_ptr        = fleet.status.enemy_ptr;
            p.owner_ptr        = &sh;
            p.strikes          = 0;
            p.kill             = env.getTorpedoKillPower(sh.data.torpedoType);
            p.expl             = env.getTorpedoDamagePower(sh.data.torpedoType);
            p.death_flag       = p.expl;
            p.speed            = FLAK_TORP_MOVEMENT_SPEED;
            if (!m_alternativeCombat) {
                p.kill *= 2, p.expl *= 2;
            }
            if (random(100) < sh.config.TorpHitOdds) {
                p.strikes = 1;
            }

            sh.status.torpedoCharge[tl] = 0;
            sh.status.numTorpedoes--;

            vis.createTorpedo(p.visId, p.position, sh.data.player, Ship::getShipNumber(p.enemy_ptr));
            goto next_ship;
        }
     next_ship:;
    }
}

/* Fire all beams from fleet. */
void
game::vcr::flak::Algorithm::fireBeams(const Fleet& fleet, const Environment& env, Visualizer& vis)
{
    // ex FlakBattle::fireBeams, flak.pas:FireBeams
    if (!fleet.isAlive()) {
        return;
    }

    const double dist = fleet.status.enemy_ptr
        ? fleet.status.position.distanceTo(fleet.status.enemy_ptr->fleetLink.status.position)
        : 1.0e+15;

    const size_t last_ship = fleet.data.firstShipIndex + fleet.data.numShips;
    for (size_t sh = fleet.data.firstShipIndex; sh < last_ship; ++sh) {
        Ship& ship = *m_ships[sh];
        if (!ship.isAlive()) {
            continue;
        }

        /* first, try to fire at fighters */
        for (int bm = 0; bm < ship.data.numBeams; ++bm) {
            if (ship.status.beamCharge[bm] >= ship.config.BeamHitFighterCharge) {
                /* find a fighter */
                Object* min_ftr = 0;
                double min_dist = 0;
                for (size_t i = 0; i < m_playerIndex.size(); ++i) {
                    Player& pl = *m_playerIndex[i];
                    // FIXME: PCC1 checks '&& have_any_fighters' as additional optimisation
                    if (pl.number == ship.data.player) {
                        continue;
                    }
                    for (size_t pi = pl.stuff.size(); pi > 0; --pi) {
                        Object* p = pl.stuff[pi-1];
                        if (p->kind == oFighter
                            && p->enemy_ptr && p->owner_ptr && fleet.status.enemy_ptr
                            && (&p->enemy_ptr->fleetLink == &ship.fleetLink
                                || &p->owner_ptr->fleetLink == &fleet.status.enemy_ptr->fleetLink))
                        {
                            double fdist = fleet.status.position.distanceTo(p->position);
                            if (fdist <= ship.config.BeamFiringRange
                                && (!min_ftr
                                    || (m_fireOnAttackFighters
                                        && min_ftr->strikes == 0 && p->strikes > 0)
                                    || (fdist <= min_dist
                                        && (!m_fireOnAttackFighters
                                            || (min_ftr->strikes == 0 && p->strikes == 0)
                                            || (min_ftr->strikes > 0 && p->strikes > 0)))))
                            {
                                min_ftr = p;
                                min_dist = fdist;
                            }
                        }
                    }
                }

                /* got a fighter? */
                if (min_ftr) {
                    if (random(100) < ship.config.BeamHitOdds) {
                        vis.fireBeamShipFighter(sh, bm, min_ftr->visId, true);
                        vis.killFighter(min_ftr->visId);
                        min_ftr->kind = oDeleteMe;
                        if (min_ftr->owner_ptr) {
                            --min_ftr->owner_ptr->status.numFightersLaunched;
                        }
                    } else {
                        vis.fireBeamShipFighter(sh, bm, min_ftr->visId, false);
                    }
                    ship.status.beamCharge[bm] = 0;
                    goto next_ship;
                }
            } /* if beam charged */
        } /* for each beam */

        if (fleet.status.enemy_ptr && fleet.status.enemy_ptr->isAlive()
            && dist <= ship.config.BeamFiringRange)
        {
            for (int bm = 0; bm < ship.data.numBeams; ++bm) {
                if (ship.status.beamCharge[bm] >= ship.config.BeamHitShipCharge) {
                    int kill = env.getBeamKillPower(ship.data.beamType);
                    int damage = env.getBeamDamagePower(ship.data.beamType);
                    if (env.getPlayerRaceNumber(ship.data.player) == 5) {
                        kill *= 3;
                    }
                    if (random(100) < ship.config.BeamHitOdds) {
                        vis.fireBeamShipShip(sh, bm, Ship::getShipNumber(fleet.status.enemy_ptr), true);
                        hitShipWith(*fleet.status.enemy_ptr, ship,
                                    damage * int32_t(ship.status.beamCharge[bm]) / 1000,
                                    kill * int32_t(ship.status.beamCharge[bm]) / 1000,
                                    damage /* death flag */);
                    } else {
                        vis.fireBeamShipShip(sh, bm, Ship::getShipNumber(fleet.status.enemy_ptr), false);
                    }
                    ship.status.beamCharge[bm] = 0;
                    goto next_ship;
                }
            }
        }

     next_ship:;
    }
}

/** Check whether battle has ended.
    \return true if battle ends. */
bool
game::vcr::flak::Algorithm::endCheck() const
{
    // ex FlakBattle::endCheck, flak.pas:FlakEndCheck
    for (size_t i = 0; i < m_playerIndex.size(); ++i) {
        if (!m_playerIndex[i]->stuff.empty()) {
            return false;
        }
    }
    for (size_t i = 0; i < m_fleets.size(); ++i) {
        if (m_fleets[i]->status.alive && m_fleets[i]->status.enemy_ptr) {
            return false;
        }
    }
    return true;
}

/** Compute new position for a fleet.
    Modifies the fleet in-place.
    This computes the newPosition field, it does not actually move the fleet yet. */
void
game::vcr::flak::Algorithm::computeNewPosition(Fleet& fleet, const Environment& env, Visualizer& vis, size_t fleetNr)
{
    // ex FlakBattle::computeNewPosition, flak.pas:ComputeNewPosition
    fleet.newPosition = fleet.status.position;

    if (!fleet.isAlive()) {
        return;
    }

    /* if our enemy died, pick a new one */
    if (fleet.status.enemy_ptr && !fleet.status.enemy_ptr->isAlive()) {
        chooseEnemy(fleet, env, vis, fleetNr);
    }

    /* FIXME: this is not quite clean because it gets the StandoffDistance
       from the ship. Better idea? */
    const int sod = m_ships[fleet.data.firstShipIndex]->config.StandoffDistance;
    if (fleet.status.enemy_ptr) {
        /* move towards target */
        Position target = fleet.status.enemy_ptr->fleetLink.status.position;

        double dist = fleet.status.position.distanceTo(target);
        if (dist > 0) {
            double nd = dist - fleet.data.speed;
            if (nd < sod) {
                nd = sod;
            }

            if (nd < dist) {
                fleet.newPosition.x = target.x + util::roundToInt((fleet.status.position.x - target.x) * nd / dist);
                fleet.newPosition.y = target.y + util::roundToInt((fleet.status.position.y - target.y) * nd / dist);
                fleet.newPosition.z = 0;
            }
        }
    }

    /* 'bouncing'. I know this is physical bullshit, but it looks nice. */
    const double speed = 0.75;
    int32_t bvx = 0, bvy = 0;
    double bvdivi = 0;

    int player = fleet.data.player;

    for (size_t ff = 0; ff < m_fleets.size(); ++ff) {
        if (m_fleets[ff] == &fleet || !m_fleets[ff]->isAlive()) {
            continue;
        }

        double idist = m_fleets[ff]->status.position.distanceTo(fleet.newPosition);
        int    their_player = m_fleets[ff]->data.player;
        if (idist > 10 && ((player == their_player ? 4*idist : 2*idist) < sod)) {
            bvx += fleet.newPosition.x - m_fleets[ff]->status.position.x;
            bvy += fleet.newPosition.y - m_fleets[ff]->status.position.y;
            bvdivi += idist;
        }
    }

    if (bvx || bvy) {
        fleet.newPosition.x = util::roundToInt(fleet.newPosition.x + bvx*speed*fleet.data.speed / bvdivi);
        fleet.newPosition.y = util::roundToInt(fleet.newPosition.y + bvy*speed*fleet.data.speed / bvdivi);
    }
}

/** Fleet GC.
    Deletes all ships killed this tick, and marks the fleet dead if it happens. */
void
game::vcr::flak::Algorithm::doFleetGC(Fleet& fleet, const Environment& env, Visualizer& vis, size_t fleetNr)
{
    // ex FlakBattle::doFleetGC, flak.pas:FleetGC
    if (!fleet.isAlive()) {
        return;
    }

    /* kill all dead ships */
    const int limit = env.getPlayerRaceNumber(fleet.data.player) == 2 ? 150 : 99;
    bool alive = false;
    bool any_torps = false;
    int torps[NUM_TORPS];
    for (size_t i = 0; i < fleet.data.numShips; ++i) {
        Ship& sh = *m_ships[i + fleet.data.firstShipIndex];
        if (sh.isAlive()) {
            if (sh.status.damage > limit || (!sh.isPlanet() && sh.status.crew < 0.5)) {
                vis.killShip(i + fleet.data.firstShipIndex);
                sh.status.isAlive = false;
                --m_playerStatus[sh.data.player-1]->num_live_ships;
                m_playerStatus[sh.data.player-1]->sum_strength -= sh.data.compensation;
                if (sh.data.torpedoType > 0 && sh.data.torpedoType <= NUM_TORPS && sh.status.numTorpedoes > 0) {
                    if (!any_torps) {
                        for (int x = 0; x < NUM_TORPS; ++x) {
                            torps[x] = 0;
                        }
                    }
                    any_torps = true;
                    torps[sh.data.torpedoType-1] += sh.status.numTorpedoes;
                }
            } else {
                alive = true;
            }
        }
    }
    if (!alive) {
        vis.killFleet(fleetNr);
    }
    fleet.status.alive = alive;
    if (alive && any_torps) {
        for (size_t i = 0; i < fleet.data.numShips; ++i) {
            Ship& sh = *m_ships[i + fleet.data.firstShipIndex];
            if (sh.isAlive() && sh.data.torpedoType > 0 && sh.data.torpedoType <= NUM_TORPS) {
                sh.status.numReceivedTorpedoes += torps[sh.data.torpedoType-1] / int(fleet.data.numShips);
            }
        }
    }
}

/* Do fighter-intercept phase for two players. */
void
game::vcr::flak::Algorithm::fighterIntercept(Player& a, Player& b, Visualizer& vis)
{
    // ex FlakBattle::fighterIntercept, flak.pas:FighterIntercept
    if (a.FighterKillOdds == 0 && b.FighterKillOdds == 0) {
        return;
    }

    for (size_t ia = a.stuff.size(); ia > 0; --ia) {
        Object* pa = a.stuff[ia-1];
        if (pa->kind == oFighter) {
            for (size_t ib = b.stuff.size(); ib > 0; --ib) {
                Object* pb = b.stuff[ib-1];
                if (pb->kind == oFighter) {
                    /* two fighters. Possible targets? */
                    if (pa->owner_ptr != 0 && pb->owner_ptr != 0 &&
                        (pa->enemy_ptr == pb->owner_ptr
                         || pb->enemy_ptr == pa->owner_ptr))
                    {
                        if (tryIntercept(*pa, *pb, vis)) {
                            return;
                        }
                        if (pa->kind != oFighter) {
                            break;  /* can this happen? I think no. */
                        }
                    }
                }
            }
        }
    }
}

/* Attempt fighter-intercept between two fighters.
   \param pa,pb  two fighters
   \return true if successful, false if no fighter intercept here */
bool
game::vcr::flak::Algorithm::tryIntercept(Object& pa, Object& pb, Visualizer& vis)
{
    // ex FlakBattle::tryIntercept, flak.pas:FighterIntercept.TryIntercept
    /* FIXME: I'm not sure that the probabilities are correct. This
       yields one_f==0 if both are 100. */
    if (!pa.position.isDistanceLERadius(pb.position, FLAK_FIGHTER_INTERCEPT_RANGE)) {
        return false;
    }

    int left_f  = (100 - pa.owner_ptr->config.FighterKillOdds) * pb.owner_ptr->config.FighterKillOdds;
    int right_f = (100 - pb.owner_ptr->config.FighterKillOdds) * pa.owner_ptr->config.FighterKillOdds;
    int one_f   = (left_f + right_f) / 100;

    if (one_f == 0) {
        return false;
    }

    int right_probab = right_f / one_f;

    if (random(100) >= one_f) {
        return false;
    }

    if (random(100) < right_probab) {
        /* a killed */
        vis.fireBeamFighterFighter(pb.visId, pa.visId, true);
        vis.killFighter(pa.visId);
        pa.kind = oDeleteMe;
        if (pa.owner_ptr) {
            pa.owner_ptr->status.numFightersLaunched--;
        }
        addFlakLog("Fighter Intercept A Killed");
    } else {
        /* b killed */
        vis.fireBeamFighterFighter(pa.visId, pb.visId, true);
        vis.killFighter(pb.visId);
        pb.kind = oDeleteMe;
        if (pb.owner_ptr) {
            pb.owner_ptr->status.numFightersLaunched--;
        }
        addFlakLog("Fighter Intercept B Killed");
    }
    return true;
}

/* Fighters of a player fire. */
void
game::vcr::flak::Algorithm::fightersFire(const Player& player, Visualizer& vis) const
{
    // ex FlakBattle::fightersFire, flak.pas:FightersFire
    for (size_t ip = player.stuff.size(); ip > 0; --ip) {
        Object* p = player.stuff[ip-1];
        if (p->kind == oFighter && p->strikes && p->enemy_ptr->isAlive()) {
            if (p->position.isDistanceLERadius(p->enemy_ptr->fleetLink.status.position,
                                               p->owner_ptr->config.FighterFiringRange))
            {
                vis.fireBeamFighterShip(p->visId, Ship::getShipNumber(p->enemy_ptr), true);
                hitShipWith(*p->enemy_ptr, *p->owner_ptr, p->kill, p->expl, p->death_flag);
                p->strikes--;
                p->canChangeEnemy = false;
            }
        }
    }
}

/** Find new base for a fighter.
    \param player      the player whose fighter we talk about
    \param fighter     the fighter which needs a new base */
void
game::vcr::flak::Algorithm::findNewBase(const Player& player, Object& fighter) const
{
    // ex FlakBattle::findNewBase, flak.pas:FindNewBase
    double min_dist = 1.0E+15;
    Ship*  min_ship = 0;

    // FIXME: somehow, we should limit this search to the player's
    // fleets to improve performance
    for (size_t i = 0; i < m_fleets.size(); ++i) {
        if (m_fleets[i]->data.player != player.number) {
            continue;
        }
        double this_dist = m_fleets[i]->status.position.distanceTo(fighter.position);
        if (this_dist < min_dist) {
            int max_mass = 0;
            const size_t last = m_fleets[i]->data.firstShipIndex + m_fleets[i]->data.numShips;
            for (size_t j = m_fleets[i]->data.firstShipIndex; j < last; ++j) {
                Ship& sh = *m_ships[j];
                if (sh.isAlive() && sh.data.numBays && sh.data.mass > max_mass) {
                    max_mass = sh.data.mass;
                    min_dist = this_dist;
                    min_ship = &sh;
                }
            }
        }
    }
    if (min_ship) {
        min_ship->status.numFightersLaunched++;
    }
    fighter.owner_ptr = min_ship;
}

/* Move all objects belonging to a player. */
void
game::vcr::flak::Algorithm::moveStuff(Player& player, Visualizer& vis)
{
    // ex FlakBattle::moveStuff, flak.pas:MoveStuff
    for (size_t ip = player.stuff.size(); ip > 0; --ip) {
        Object& p = *player.stuff[ip-1];
        if (p.kind == oTorpedo) {
            if (moveObjectTowards(p, p.enemy_ptr->getPos()) == 0) {
                /* torp hits */
                if (p.strikes != 0) {
                    Ship* ene = p.enemy_ptr;
                    if (!ene->isAlive() && ene->fleetLink.isAlive()) {
                        ene = m_ships[ene->fleetLink.data.firstShipIndex + size_t(random(int(ene->fleetLink.data.numShips)))];
                    }
                    if (ene->isAlive()) {
                        p.enemy_ptr = ene;
                        vis.hitTorpedo(p.visId, Ship::getShipNumber(ene));
                        hitShipWith(*ene, *p.owner_ptr, p.expl, p.kill, p.death_flag);
                        p.owner_ptr->status.stat.handleTorpedoHit();
                        addFlakLog("Torp hitting unit");
                    } else {
                        p.enemy_ptr = 0;
                        vis.missTorpedo(p.visId);
                        addFlakLog("Torp hitting void");
                    }
                } else {
                    addFlakLog("Torp missing");
                    vis.missTorpedo(p.visId);
                }
                p.kind = oDeleteMe;
            } else {
                vis.moveTorpedo(p.visId, p.position);
            }
        } else if (p.kind == oFighter) {
            if (p.strikes != 0 && !p.enemy_ptr->isAlive()) {
                /* enemy died. */
                if (!p.canChangeEnemy) {
                    /* we cannot change our enemy. Retreat. */
                    addFlakLog("Fighter retreats after shooting");
                    p.strikes = 0;
                } else if (p.owner_ptr != 0 && p.owner_ptr->isAlive()) {
                    /* base is still alive. Ask it what to do. */
                    if (p.owner_ptr->fleetLink.status.enemy_ptr != p.enemy_ptr) {
                        addFlakLog("Fighter changing target");
                        p.enemy_ptr = p.owner_ptr->fleetLink.status.enemy_ptr;
                        p.strikes >>= 1;
                    } else {
                        /* base has not decided yet. Wait. */
                        addFlakLog("Fighter waiting for base");
                        p.strikes--;
                    }
                } else {
                    /* base died. Pick new one and ask it what to do. */
                    addFlakLog("Fighter picking new base");
                    findNewBase(player, p);
                    if (p.owner_ptr != 0) {
                        p.enemy_ptr = p.owner_ptr->fleetLink.status.enemy_ptr;
                    } else {
                        p.enemy_ptr = 0;
                    }
                }
            }

            if (!p.enemy_ptr || player.num_live_ships == 0) {
                p.strikes = 0;
            }

            if (p.strikes == 0) {
                /* return to base */
                if (!p.owner_ptr || !p.owner_ptr->isAlive()) {
                    findNewBase(player, p);
                }

                if (!p.owner_ptr) {
                    vis.landFighter(p.visId);
                    p.kind = oDeleteMe;
                } else {
                    if (moveObjectTowards(p, p.owner_ptr->getPos()) == 0) {
                        /* reached base */
                        vis.landFighter(p.visId);
                        p.kind = oDeleteMe;
                        p.owner_ptr->status.numFighters++;
                        p.owner_ptr->status.numFightersLaunched--;
                    } else {
                        vis.moveFighter(p.visId, p.position, Ship::getShipNumber(p.owner_ptr));
                    }
                }
            } else {
                /* move towards enemy */
                moveObjectTowards(p, p.enemy_ptr->getPos());
                vis.moveFighter(p.visId, p.position, Ship::getShipNumber(p.enemy_ptr));
            }
        }
    }
}


/*
 *  Misc
 */

void
game::vcr::flak::Algorithm::renderAll(Visualizer& vis) const
{
    // Render all fleets
    for (size_t i = 0; i < m_fleets.size(); ++i) {
        const Fleet& f = *m_fleets[i];
        vis.createFleet(i, f.status.position.x, f.status.position.y, f.data.player, f.data.firstShipIndex, f.data.numShips);
    }

    // Render all ships
    for (size_t i = 0; i < m_ships.size(); ++i) {
        const Ship& sh = *m_ships[i];
        Visualizer::ShipInfo info;
        info.name         = sh.data.name;
        info.isPlanet     = sh.isPlanet();
        info.player       = sh.data.player;
        info.shield       = getShield(i);     // use public method for rounding
        info.damage       = getDamage(i);
        info.crew         = getCrew(i);
        info.numBeams     = sh.data.numBeams;
        info.numLaunchers = sh.data.numLaunchers;
        info.numTorpedoes = sh.status.numTorpedoes;
        info.numBays      = sh.data.numBays;
        info.numFighters  = sh.status.numFighters;
        info.torpedoType  = sh.data.torpedoType;
        info.beamType     = sh.data.beamType;
        info.mass         = sh.data.mass;
        info.id           = sh.data.id;
        vis.createShip(i, sh.getPos(), info);
    }
}


/*
 *  Debugging
 */

#if FLAK_CHECKPOINT
namespace {
    void writeUnit(std::ostream& os, const game::vcr::flak::Algorithm::Ship* p)
    {
        if (!p) {
            os << "none";
        } else {
            os << "#" << p->data.id;
        }
    }
}

/** Print a checkpoint.
    This function is used for debugging.
    It writes out almost all state variables to the specified stream.
    A similar checkpoint function is available in the DOS FLAK player;
    we can thus generate traces of a fight which must be identical.
    Be careful to keep this output constant! */
void
game::vcr::flak::Algorithm::printCheckpoint(std::ostream& os)
{
    // ex FlakBattle::printCheckpoint, flak.pas:FlakPrintCheckpoint
    os << "=== " << m_time << " (seed=" << int32_t(m_seed) << ") ===\n";
    for (size_t i = 0; i < m_fleets.size(); ++i) {
        const Fleet& f = *m_fleets[i];
        os << "fleet " << i << ": pos=" << f.status.position.x << "," << f.status.position.y
           << " alive="
           << (f.isAlive() ? "TRUE" : "FALSE")
           << " enemy=";
        writeUnit(os, f.status.enemy_ptr);
        os << "\n";
    }
    for (size_t i = 0; i < m_ships.size(); ++i) {
        const Ship& s = *m_ships[i];
        os << "ship " << i << ": crew=" << int(s.status.crew)
           << " damage=" << int(s.status.damage)
           << " shield=" << int(s.status.shield)
           << " alive=" << (s.isAlive() ? "TRUE": "FALSE")
           << " bs=";
        for (int j = 0; j < 5; ++j) {
            os << s.status.beamCharge[j] << " ";
        }
        os << "ts=";
        for (int j = 0; j < 5; ++j) {
            os << s.status.torpedoCharge[j] << " ";
        }
        os << "fs=";
        for (int j = 0; j < 5; ++j) {
            os << s.status.bayCharge[j] << " ";
        }
        os << "ftrs_out=" << s.status.numFightersLaunched
           << " ftrs_in=" << s.status.numFighters
           << " cdn=" << s.status.launchCountdown
           << " torps=" << s.status.numTorpedoes
           << "\n";
    }
    for (int i = 1; i <= 11; ++i) {
        if (size_t(i-1) < m_playerStatus.size()) {
            const Player& pl = *m_playerStatus[i-1];
            os << "player " << i << ": num_live_ships="
               << pl.num_live_ships << "\n";
            for (size_t oi = pl.stuff.size(); oi > 0; --oi) {
                Object* o = pl.stuff[oi-1];
                os << "  " << int(o->kind) << " at "
                   << o->position.x << "," << o->position.y << " ene=";
                writeUnit(os, o->enemy_ptr);
                os << " owner=";
                writeUnit(os, o->owner_ptr);
                os << " kill=" << o->kill << " expl=" << o->expl
                   << " speed=" << o->speed << " strikes=" << o->strikes
                   << " can_change=" << (o->canChangeEnemy ? "TRUE":"FALSE")
                   << "\n";
            }
        } else {
            os << "player " << i << ": num_live_ships=0\n";
        }
    }
}
#endif
