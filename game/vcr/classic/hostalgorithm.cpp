/**
  *  \file game/vcr/classic/hostalgorithm.cpp
  */

#include <algorithm>
#include "game/vcr/classic/hostalgorithm.hpp"
#include "game/vcr/classic/visualizer.hpp"
#include "game/vcr/classic/statustoken.hpp"
#include "util/math.hpp"

/* Benchmark results: 'testvcr tests/vcr/vcr2.dat . 100', vcr2.dat from Manos-II / 35 on roadrunner (Pentium 200 MHz, gcc-2.95, i386):

   + obfuscated seed advance:  2.350 user
   + 1-side instead of !side:  2.480 user
   + rand_table_type=uchar:    2.520 user
   + original:                 2.540 user
   + #undef CONF_STATIC_VCR:   2.950 user

   New results, 'testvcr tests/vcr/vcr2.dat specs 1000' on royale (Athlon 2000 MHz, gcc-3.3, i386):

   + starting with:            2.840
   + skip fireBeamsAtFighter:  2.760
   + fighter_status_type=u8:   2.740
   + 20110917:                 2.580
   + some inlines, rdivadd:    2.370
   + 20120612:                 2.210

   Removing VCR_STATIC, but further optimisations (in particular, store GVcrObject in Status and pass Status& around):
   + 20120616                  2.160

   Fast access paths (f_getDamage/f_getKill) seems not to get much.

   New new results, 'time ./testvcr tests/vcr/vcr2.dat specs 10000' on rocket (i5 3700 MHz, gcc-4.9, x64):
   + PCC2 20160323             2.90
   + ... with gcc-3.3 i386     6.34 (for comparison)
   + C2NG 20160326             2.95 */

namespace {
    const int RANDOM_SIZE = 119;

    typedef uint8_t RandomTable_t;

    const RandomTable_t RANDOM_TABLE_1_20[RANDOM_SIZE] = {
   	9,	8,	11,	8,	5,	5,
	9,	10,	15,	2,	10,	4,	14,
	18,	1,	14,	15,	17,	2,	4,
	10,	13,	16,	17,	11,	10,	14,
	7,	2,	8,	13,	13,	18,	6,
	13,	12,	6,	12,	6,	14,	4,
	1,	20,	16,	16,	2,	8,	10,
	18,	4,	20,	16,	17,	15,	6,
	19,	16,	14,	2,	15,	11,	6,
	9,	17,	15,	4,	3,	12,	16,
	19,	12,	18,	11,	13,	13,	8,
	3,	2,	15,	5,	12,	6,	10,
	6,	9,	16,	20,	19,	18,	17,
	11,	1,	4,	12,	7,	13,	15,
	5,	7,	12,	3,	3,	7,	14,
	10,	18,	13,	3,	16,	14,	4,
	13,	9,	14,	2,	9,	7,	4,	15
    };

    const RandomTable_t RANDOM_TABLE_1_100[RANDOM_SIZE] = {
	42,	36,	54,	39,	23,	21,
	41,	45,	73,	5,	47,	14,	71,
	89,	2,	70,	76,	83,	5,	16,
	50,	64,	78,	87,	53,	47,	66,
	33,	5,	37,	63,	61,	88,	29,
	62,	58,	26,	61,	30,	67,	16,
	2,	98,	78,	81,	7,	37,	46,
	88,	15,	99,	77,	82,	75,	25,
	96,	79,	69,	5,	71,	54,	25,
	43,	87,	75,	17,	13,	58,	78,
	96,	57,	87,	52,	63,	64,	36,
	14,	5,	73,	23,	58,	29,	48,
	27,	43,	77,	99,	95,	88,	84,
	55,	2,	15,	57,	33,	61,	76,
	22,	31,	61,	11,	13,	31,	70,
	45,	92,	61,	11,	80,	71,	14,
	62,	44,	70,	4,	40,	32,	18,	74
    };

    const RandomTable_t RANDOM_TABLE_1_17[RANDOM_SIZE] = {
	8,	7,	10,	7,	5,	4,
	7,	8,	13,	2,	8,	3,	12,
	15,	1,	12,	13,	14,	2,	3,
	9,	11,	13,	15,	9,	8,	12,
	6,	2,	7,	11,	11,	15,	5,
	11,	10,	5,	11,	6,	12,	3,
	1,	17,	14,	14,	2,	7,	8,
	15,	3,	17,	13,	14,	13,	5,
	16,	14,	12,	2,	12,	10,	5,
	8,	15,	13,	4,	3,	10,	13,
	16,	10,	15,	9,	11,	11,	7,
	3,	2,	13,	5,	10,	5,	9,
	5,	8,	13,	17,	16,	15,	14,
	10,	1,	3,	10,	6,	11,	13,
	4,	6,	11,	3,	3,	6,	12,
	8,	16,	11,	3,	14,	12,	3,
	11,	8,	12,	2,	7,	6,	4,	13
    };
}

/**************************** HostStatusToken ****************************/

class game::vcr::classic::HostAlgorithm::HostStatusToken : public game::vcr::classic::StatusToken {
 public:
    HostStatusToken(const HostAlgorithm& parent);
    void restore(HostAlgorithm& parent) const;
    Status m_leftStatus;
    Status m_rightStatus;
    int m_seed;
    BattleResult_t m_result;
    int m_nuFlag;
};

inline
game::vcr::classic::HostAlgorithm::HostStatusToken::HostStatusToken(const HostAlgorithm& parent)
    : StatusToken(parent.m_time),
      m_leftStatus(parent.m_status[LeftSide]),
      m_rightStatus(parent.m_status[RightSide]),
      m_seed(parent.m_seed),
      m_result(parent.m_result),
      m_nuFlag(parent.m_nuFlag)
{ }

inline void
game::vcr::classic::HostAlgorithm::HostStatusToken::restore(HostAlgorithm& parent) const
{
    parent.m_time              = getTime();
    parent.m_status[LeftSide]  = m_leftStatus;
    parent.m_status[RightSide] = m_rightStatus;
    parent.m_seed              = m_seed;
    parent.m_result            = m_result;
    parent.m_nuFlag            = m_nuFlag;
}

/********************************* Status ********************************/

void
game::vcr::classic::HostAlgorithm::Status::init(const Object& obj, Side side)
{
    // ex VcrPlayerTHost::Status::clear
    for (int i = 0; i < VCR_MAX_BEAMS; ++i) {
        m_beamStatus[i] = 0;
    }
    for (int i = 0; i < VCR_MAX_TORPS; ++i) {
        m_launcherStatus[i] = 0;
    }
    for (int i = 0; i < VCR_MAX_FTRS; ++i) {
        m_fighterX[i] = 0;
        m_fighterStatus[i] = FighterIdle;
    }
    m_objectX = 0;
    m_damageLimit = 100;
    m_numFightersOut = 0;
    m_side = side;
    m_obj = obj;
    m_obj.setShield(std::max(0, std::min(m_obj.getShield(), 100 - m_obj.getDamage())));
}

/***************************** HostAlgorithm *****************************/

// Constructor.
game::vcr::classic::HostAlgorithm::HostAlgorithm(bool nuFlag,
                                                 Visualizer& vis,
                                                 const game::config::HostConfiguration& config,
                                                 const game::spec::ShipList& list)
    : Algorithm(vis),
      m_config(config),
      m_shipList(list),
      m_nuFlag(nuFlag),
      m_seed(0),
      m_time(0),
      m_result()
{ }

// Destructor.
game::vcr::classic::HostAlgorithm::~HostAlgorithm()
{ }

bool
game::vcr::classic::HostAlgorithm::checkBattle(Object& left, Object& right, uint16_t& seed)
{
    // ex VcrPlayerTHost::checkVcr
    bool leftResult = checkSide(left);
    bool rightResult = checkSide(right);

    // unsure about this one, but it cannot hurt.
    bool seedResult = false;
    if (seed > RANDOM_SIZE) {
        seed = 1;
        seedResult = true;
    }

    return leftResult || rightResult || seedResult;
}

// /** Init. Set up for playing. */
void
game::vcr::classic::HostAlgorithm::initBattle(const Object& left, const Object& right, uint16_t seed)
{
    // ex VcrPlayerTHost::initVcr
    m_result = BattleResult_t();

    Object leftCopy(left);
    Object rightCopy(right);
    if (checkBattle(leftCopy, rightCopy, seed)) {
        m_result += Invalid;
        return;
    }

    // okay, we'll be able to play it.
    m_seed = seed % RANDOM_SIZE;

    // clear status
    m_status[LeftSide].init(leftCopy, LeftSide);
    m_status[RightSide].init(rightCopy, RightSide);
    m_statistic[LeftSide] = leftCopy;
    m_statistic[RightSide] = rightCopy;

    m_status[LeftSide].m_objectX = 30;
    if (!m_status[RightSide].m_obj.isPlanet()) {
        m_status[RightSide].m_objectX = 610;
    } else {
        m_status[RightSide].m_objectX = 570;
    }

    // Left shield
    if (isFreighter(m_status[LeftSide])) {
        m_status[LeftSide].m_obj.setShield(0);
    }

    // Right shield
    if (!m_status[RightSide].m_obj.isPlanet()) {
        if (isFreighter(m_status[RightSide])) {
            m_status[RightSide].m_obj.setShield(0);
        }
    } else {
        if (m_status[RightSide].m_obj.getCrew() <= 0) {
            m_status[RightSide].m_obj.setShield(0);
        }
    }

    preloadWeapons(m_status[LeftSide]);
    preloadWeapons(m_status[RightSide]);

    m_status[LeftSide].m_damageLimit  = (m_config.getPlayerRaceNumber(m_status[LeftSide].m_obj.getOwner()) == 2) ? 151 : 100;
    m_status[RightSide].m_damageLimit = (m_config.getPlayerRaceNumber(m_status[RightSide].m_obj.getOwner()) == 2) ? 151 : 100;
    if (m_nuFlag && m_status[RightSide].m_obj.isPlanet()) {
        m_status[RightSide].m_damageLimit = 100;
    }

    m_time = 0;
    // FIXME: port these:
    //     vis->init();
}

// /** Finish up VCR. Take back fighters, explode, compute status. */
void
game::vcr::classic::HostAlgorithm::doneBattle(Object& left, Object& right)
{
    // ex VcrPlayerTHost::doneVcr
//     ASSERT(status_word != VCRS_INVALID);

    // FIXME: explode fighters when mothership explodes -- NOT! too expensive in visualisation
    for (int i = 0; i < VCR_MAX_FTRS; ++i) {
        if (m_status[LeftSide].m_fighterStatus[i] != FighterIdle) {
            m_status[LeftSide].m_obj.addFighters(+1);
            visualizer().landFighter(LeftSide, i);
            m_status[LeftSide].m_fighterStatus[i] = FighterIdle;
            --m_status[LeftSide].m_numFightersOut;
        }
        if (m_status[RightSide].m_fighterStatus[i] != FighterIdle) {
            m_status[RightSide].m_obj.addFighters(+1);
            visualizer().landFighter(RightSide, i);
            m_status[RightSide].m_fighterStatus[i] = FighterIdle;
            --m_status[RightSide].m_numFightersOut;
        }
    }

    m_result = BattleResult_t();
    if (m_status[RightSide].m_obj.isPlanet()) {
        // FIXME: it seems that the ship can capture the planet even if it explodes in HOST. This affects the simulator.
        if (m_status[LeftSide].m_obj.getDamage() >= 100 || m_status[LeftSide].m_obj.getCrew() <= 0) {
            m_result += LeftDestroyed;
        }
        if (m_status[RightSide].m_obj.getDamage() >= 100) {
            m_result += RightDestroyed;
        }
    } else {
        if (m_status[LeftSide].m_obj.getDamage() >= m_status[LeftSide].m_damageLimit) {
            m_result += LeftDestroyed;
        } else if (m_status[LeftSide].m_obj.getCrew() <= 0) {
            if (m_status[LeftSide].m_obj.getDamage() < m_status[RightSide].m_damageLimit) {
                m_result += LeftCaptured;
            } else {
                m_result += LeftDestroyed;
            }
        }
        if (m_status[RightSide].m_obj.getDamage() >= m_status[RightSide].m_damageLimit) {
            m_result += RightDestroyed;
        } else if (m_status[RightSide].m_obj.getCrew() <= 0) {
            if (m_status[RightSide].m_obj.getDamage() < m_status[LeftSide].m_damageLimit) {
                m_result += RightCaptured;
            } else {
                m_result += RightDestroyed;
            }
        }
    }

    if (m_result.empty()) {
        m_result += Timeout;
    }

    if (m_result.contains(LeftDestroyed)) {
        visualizer().killObject(LeftSide);
    }
    if (m_result.contains(RightDestroyed)) {
        visualizer().killObject(RightSide);
    }

    left = m_status[LeftSide].m_obj;
    right = m_status[RightSide].m_obj;
}

bool
game::vcr::classic::HostAlgorithm::setCapabilities(uint16_t cap)
{
    // ex VcrPlayerTHost::setCapabilities
    return cap == 0;
}

// /** Play one cycle. */
bool
game::vcr::classic::HostAlgorithm::playCycle()
{
    // ex VcrPlayerTHost::playCycle
    if (!m_result.empty()) {
        return false;
    }

    if (m_status[LeftSide].m_obj.getDamage() >= m_status[LeftSide].m_damageLimit || m_status[RightSide].m_obj.getDamage() >= m_status[RightSide].m_damageLimit) {
        return false;
    }
    if (m_status[LeftSide].m_obj.getCrew() <= 0 || m_status[RightSide].m_obj.getCrew() <= 0) {
        if (!m_status[RightSide].m_obj.isPlanet()) {
            return false;
        }
    }
    if (m_time >= 2000) {
        return false;
    }

    ++m_time;

    // Movement
    int distance = m_status[RightSide].m_objectX - m_status[LeftSide].m_objectX;
    if (distance > 30) {
        ++m_status[LeftSide].m_objectX;
        --distance;
        if (!m_status[RightSide].m_obj.isPlanet()) {
            --m_status[RightSide].m_objectX;
            --distance;
        }
    }

    // Beams
    if (distance < 200) {
        fireBeams(m_status[LeftSide], m_status[RightSide]);
    }
    fireBeamsAtFighter(m_status[LeftSide], m_status[RightSide]);
    fireBeamsAtFighter(m_status[RightSide], m_status[LeftSide]);
    if (distance < 200) {
        fireBeams(m_status[RightSide], m_status[LeftSide]);
    }

    // Torpedoes
    if (distance < 300) {
        fireTorpedoes(m_status[LeftSide], m_status[RightSide]);
        fireTorpedoes(m_status[RightSide], m_status[LeftSide]);
    }

    // Fighters
    launchFighters(m_status[LeftSide]);
    launchFighters(m_status[RightSide]);
    if (m_status[LeftSide].m_numFightersOut > 0 || m_status[RightSide].m_numFightersOut > 0) {
        fighterStuff();
    }

    // Recharge beams
    rechargeBeams(m_status[LeftSide]);
    rechargeBeams(m_status[RightSide]);

    return true;
}

// /** Fast forward. In torp/torp battles, nothing happens until ships are
//     in torpedo range. So start there instead.
//     \todo the same applies to freighters */
void
game::vcr::classic::HostAlgorithm::playFastForward()
{
    // ex VcrPlayerTHost::playFastForward

    // not applicable if we already have a result, have played a tick, or are fighting a planet.
    if (!m_result.empty() || m_time != 0 || m_status[RightSide].m_obj.isPlanet()) {
        return;
    }

    // not applicable for carriers
    if (m_status[LeftSide].m_obj.getNumBays() != 0 || m_status[RightSide].m_obj.getNumBays() != 0) {
        return;
    }

    if (m_status[LeftSide].m_obj.getShield() != 100 || m_status[RightSide].m_obj.getShield() != 100) {
        return;
    }

    // Start at distance 304 (to compensate off-by-one errors, 300 should suffice ;-)
    m_status[LeftSide].m_objectX = 168;
    m_status[RightSide].m_objectX = 472;
    m_time = 138;

    m_seed = (m_seed + 138 * 2 * (m_status[LeftSide].m_obj.getNumBeams() + m_status[RightSide].m_obj.getNumBeams())) % RANDOM_SIZE;
}

int
game::vcr::classic::HostAlgorithm::getBeamStatus(Side side, int id)
{
    // ex VcrPlayerTHost::getBeamStatus
    // FIXME: range check
    return m_status[side].m_beamStatus[id];
}

int
game::vcr::classic::HostAlgorithm::getLauncherStatus(Side side, int id)
{
    // ex VcrPlayerTHost::getTorpStatus
    // scale [0,40] to [0,100]
    // FIXME: range check
    return m_status[side].m_launcherStatus[id] * 5 / 2;
}

int
game::vcr::classic::HostAlgorithm::getNumTorpedoes(Side side)
{
    // ex VcrPlayerTHost::getTorpCount
    if (m_status[side].m_obj.getNumLaunchers() != 0) {
        return m_status[side].m_obj.getNumTorpedoes();
    } else {
        return 0;
    }
}

int
game::vcr::classic::HostAlgorithm::getNumFighters(Side side)
{
    // ex VcrPlayerTHost::getFighterCount
    if (m_status[side].m_obj.getNumBays() != 0) {
        return m_status[side].m_obj.getNumFighters();
    } else {
        return 0;
    }
}

int
game::vcr::classic::HostAlgorithm::getShield(Side side)
{
    // ex VcrPlayerTHost::getShields
    return m_status[side].m_obj.getShield();
}

int
game::vcr::classic::HostAlgorithm::getDamage(Side side)
{
    // ex VcrPlayerTHost::getDamage
    return m_status[side].m_obj.getDamage();
}

int
game::vcr::classic::HostAlgorithm::getCrew(Side side)
{
    // ex VcrPlayerTHost::getCrew
    return m_status[side].m_obj.getCrew();
}

int
game::vcr::classic::HostAlgorithm::getFighterX(Side side, int id)
{
    // ex VcrPlayerTHost::getFighterX
    // FIXME: range check
    return m_status[side].m_fighterX[id];
}

game::vcr::classic::FighterStatus
game::vcr::classic::HostAlgorithm::getFighterStatus(Side side, int id)
{
    // ex VcrPlayerTHost::getFighterStatus
    // FIXME: range check
    return FighterStatus(m_status[side].m_fighterStatus[id]);
}

int
game::vcr::classic::HostAlgorithm::getObjectX(Side side)
{
    // ex VcrPlayerTHost::getObjectX
    return m_status[side].m_objectX;
}

int32_t
game::vcr::classic::HostAlgorithm::getDistance()
{
    // ex VcrPlayerTHost::getDistance
    return (m_status[RightSide].m_objectX - m_status[LeftSide].m_objectX) * int32_t(100);
}

// /** Save status. The easy mindless way -- just save everything. */
game::vcr::classic::StatusToken*
game::vcr::classic::HostAlgorithm::createStatusToken()
{
    // ex VcrPlayerTHost::getStatusToken
    return new HostStatusToken(*this);
}

// /** Restore status. */
void
game::vcr::classic::HostAlgorithm::restoreStatus(const StatusToken& token)
{
    // ex VcrPlayerTHost::setStatus
    if (const HostStatusToken* t = dynamic_cast<const HostStatusToken*>(&token)) {
        t->restore(*this);
    } else {
        // FIXME: else what?
    }
}

game::vcr::classic::Time_t
game::vcr::classic::HostAlgorithm::getTime()
{
    return m_time;
}

game::vcr::classic::BattleResult_t
game::vcr::classic::HostAlgorithm::getResult()
{
    return m_result;
}

game::vcr::Statistic
game::vcr::classic::HostAlgorithm::getStatistic(Side side)
{
    return m_statistic[side];
}





// /** Random number between 1 and 20. The infamous VCR random number
//     generator. Fetches the value from a table.

//     For those who don't know it yet: VCR.EXE has a table with 119
//     "random" numbers in the range [0,1000]. During a fight, these are
//     repeatedly used and scaled to the requested range; the seed field
//     in a VCR record is the initial index into the array (1 meaning the
//     first entry). This implementation of the VCR uses three tables
//     with pre-scaled values to avoid the costly rescale operation.
//     Although VCR.EXE actually uses five ranges, it turns out that we
//     can safely omit two of them by re-scaling the tests.

//     Our seed runs from 1 to RANDOM_SIZE, although this spelling of the
//     random number generator also allows setting it to zero (meaning
//     the same as RANDOM_SIZE). */
game::vcr::classic::HostAlgorithm::Random_t
game::vcr::classic::HostAlgorithm::getRandom_1_20()
{
    // ex VcrPlayerTHost::psrandom_1_20()
    // The obfuscated form makes gcc generate better code at -O2 (only two references to `seed', no in-memory increment)
    return RANDOM_TABLE_1_20[(m_seed = (m_seed >= RANDOM_SIZE ? 1 : m_seed+1))-1];
}

// /** Random number between 1 and 100. @see psrandom_1_20(). */
game::vcr::classic::HostAlgorithm::Random_t
game::vcr::classic::HostAlgorithm::getRandom_1_100()
{
    // ex VcrPlayerTHost::psrandom_1_100
    return RANDOM_TABLE_1_100[(m_seed = (m_seed >= RANDOM_SIZE ? 1 : m_seed+1))-1];
}

// /** Random number between 1 and 17. @see psrandom_1_20(). */
game::vcr::classic::HostAlgorithm::Random_t
game::vcr::classic::HostAlgorithm::getRandom_1_17()
{
    // ex VcrPlayerTHost::psrandom_1_17
    return RANDOM_TABLE_1_17[(m_seed = (m_seed >= RANDOM_SIZE ? 1 : m_seed+1))-1];
}

// /** Compute a/b+plus, using variable rounding.
//     - nuflag=0: IEEE rounding (nearest or even)
//     - nuflag=1: arithmetic rounding (nearest or up) */
// FIXME: rename this one?
int32_t
game::vcr::classic::HostAlgorithm::rdivadd(int32_t a, int32_t b, int32_t plus)
{
    // ex VcrPlayerTHost::rdivadd
    int32_t x = a / b + plus;
    int32_t r = a % b;
    if (r*2 + ((x&1)|m_nuFlag) > b) {
        ++x;
    }
    return x;
}

// /** Check if freighter. \return true iff side (0 or 1) is a freighter. */
// FIXME: move into Object?
bool
game::vcr::classic::HostAlgorithm::isFreighter(const Status& st)
{
    // ex VcrPlayerTHost::isFreighter
    return st.m_obj.getNumBeams() == 0
        && st.m_obj.getNumLaunchers() == 0
        && st.m_obj.getNumBays() == 0;
}

// /** Hit object.
//     \param side     who is hit
//     \param damage   `damage' (explosive) power of weapon
//     \param kill     `kill' (x-ray) power of weapon */
void
game::vcr::classic::HostAlgorithm::hit(Status& st, int damage, int kill)
{
    // ex VcrPlayerTHost::hit
    int shld = -rdivadd(80*damage, st.m_obj.getMass() + 1, 1 - st.m_obj.getShield());
    if (shld < 0) {
        st.m_obj.setShield(0);
        long l = rdivadd(-80L*shld, st.m_obj.getMass() + 1, st.m_obj.getDamage() + 1);
        if (l > 9999) {
            l = 9999;
        }
        st.m_obj.setDamage(l);
        shld = 0;
        if (st.m_obj.isPlanet() && damage > 1) {
            int beam = 10 - m_status[RightSide].m_obj.getDamage() / 10;
            // bug emulation
            if (beam <= 0) {
                if (m_status[LeftSide].m_obj.getBeamType() > 0) {
                    m_status[RightSide].m_obj.setBeamType(m_status[LeftSide].m_obj.getBeamType());
                }
            } else {
                if (m_status[RightSide].m_obj.getBeamType() > beam) {
                    m_status[RightSide].m_obj.setBeamType(beam);
                }
            }
        }
    }

    if (st.m_obj.getShield() == 0) {
        int crew = st.m_obj.getCrew();
        if (!st.m_obj.isPlanet()) {
            crew = - rdivadd(80*util::divideAndRound((100-st.m_obj.getCrewDefenseRate()) * kill, 100), st.m_obj.getMass() + 1, -crew);
            if (crew < 0) {
                st.m_obj.setCrew(0);
            } else {
                st.m_obj.setCrew(crew);
            }
        }
    }
    st.m_obj.setShield(shld);
}

// /** Start a fighter. Attempts to launch a fighter.
//     \pre Object has fighters. */
void
game::vcr::classic::HostAlgorithm::launchFighter(Status& st)
{
    // ex VcrPlayerTHost::launchFighter
    for (int i = 0; i < VCR_MAX_FTRS; ++i) {
        if (st.m_fighterStatus[i] == FighterIdle) {
            st.m_obj.addFighters(-1);
            st.m_fighterStatus[i] = FighterAttacks;
            st.m_fighterX[i] = st.m_objectX;
            ++st.m_numFightersOut;
            visualizer().startFighter(st.m_side, i);
            m_statistic[st.m_side].handleFightersAboard(st.m_obj.getNumFighters());
            return;
        }
    }
}

// /** Start fighter. Start fighter from specified side, if
//     preconditions fulfilled. */
void
game::vcr::classic::HostAlgorithm::launchFighters(Status& st)
{
    // ex VcrPlayerTHost::launchFighters
    if (st.m_obj.getNumBays() > 0) {
        int n = getRandom_1_20();
        if (n <= st.m_obj.getNumBays() && st.m_obj.getNumFighters() > 0 && st.m_numFightersOut < VCR_MAX_FTRS) {
            launchFighter(st);
        }
    }
}

// /** Fighter fires. The fighter on track i fires at its opponent,
//     if it's close enough. */
// void
void
game::vcr::classic::HostAlgorithm::fighterShoot(Status& st, Status& opp, int i)
{
    // ex VcrPlayerTHost::fighterShoot
    if (std::abs(st.m_fighterX[i] - opp.m_objectX) < 20) {
        hit(opp, 2, 2);
        visualizer().fireBeam(st.m_side, i, -1, 1, 2, 2);
    }
}

// /** Kill fighter. The fighter on track i, specified side, is killed
//     and removed from the game. */
void
game::vcr::classic::HostAlgorithm::killFighter(Status& st, int i)
{
    // ex VcrPlayerTHost::killFighter
    visualizer().killFighter(st.m_side, i);
    st.m_fighterStatus[i] = FighterIdle;
    --st.m_numFightersOut;
}

// /** Fighter stuff.

//     - Fighter movement
//     - Fighters fire at enemy
//     - Fighter intercept */
void
game::vcr::classic::HostAlgorithm::fighterStuff()
{
    // ex VcrPlayerTHost::fighterStuff()

    // This used to process all movements, then all firings.
    // However, since all these processes are 100% deterministic, order does not matter and I have fused movement and firing, for a little bit of speed.
    for (int i = 0; i < VCR_MAX_FTRS; ++i) {
        // left side
        if (m_status[LeftSide].m_fighterStatus[i] == FighterAttacks) {
            if (m_status[LeftSide].m_fighterX[i] > m_status[RightSide].m_objectX + 10) {
                m_status[LeftSide].m_fighterStatus[i] = FighterReturns;
                m_status[LeftSide].m_fighterX[i] -= 4;
            } else {
                m_status[LeftSide].m_fighterX[i] += 4;
                fighterShoot(m_status[LeftSide], m_status[RightSide], i);
            }
        } else if (m_status[LeftSide].m_fighterStatus[i] == FighterReturns) {
            if (m_status[LeftSide].m_fighterX[i] < m_status[LeftSide].m_objectX) {
                m_status[LeftSide].m_obj.addFighters(+1);
                visualizer().landFighter(LeftSide, i);
                m_status[LeftSide].m_fighterStatus[i] = FighterIdle;
                --m_status[LeftSide].m_numFightersOut;
            } else {
                m_status[LeftSide].m_fighterX[i] -= 4;
            }
        }

        // right side
        if (m_status[RightSide].m_fighterStatus[i] == FighterAttacks) {
            if (m_status[RightSide].m_fighterX[i] < m_status[LeftSide].m_objectX - 10) {
                m_status[RightSide].m_fighterStatus[i] = FighterReturns;
                m_status[RightSide].m_fighterX[i] += 4;
            } else {
                m_status[RightSide].m_fighterX[i] -= 4;
                fighterShoot(m_status[RightSide], m_status[LeftSide], i);
            }
        } else if (m_status[RightSide].m_fighterStatus[i] == FighterReturns) {
            if (m_status[RightSide].m_fighterX[i] > m_status[RightSide].m_objectX) {
                m_status[RightSide].m_obj.addFighters(+1);
                visualizer().landFighter(RightSide, i);
                m_status[RightSide].m_fighterStatus[i] = FighterIdle;
                --m_status[RightSide].m_numFightersOut;
            } else {
                m_status[RightSide].m_fighterX[i] += 4;
            }
        }
    }

    if (m_status[LeftSide].m_numFightersOut > 0 && m_status[RightSide].m_numFightersOut > 0) {
        for (int i = 0; i < VCR_MAX_FTRS; ++i) {
            if (m_status[LeftSide].m_fighterStatus[i] != FighterIdle) {
                for (int j = 0; j < VCR_MAX_FTRS; ++j) {
                    if (m_status[RightSide].m_fighterStatus[j] != FighterIdle) {
                        if (m_status[LeftSide].m_fighterX[i] == m_status[RightSide].m_fighterX[j]) {
                            // fighter intercept
                            int n = getRandom_1_100();
                            if (m_status[LeftSide].m_fighterStatus[i] == FighterIdle) {
                                // Tim is the king! A dead fighter can still fire!
                                // Our visualisation can't handle that, so use the short way.
                                // See tests/vcr/deadfire.vcr for an example fight.
                                if (n >= 50) {
                                    killFighter(m_status[RightSide], j);
                                }
                            } else {
                                /* regular fighter intercept code */
                                if (n < 50) {
                                    visualizer().fireBeam(RightSide, j, i, 1, 2, 2);
                                    killFighter(m_status[LeftSide], i);
                                } else {
                                    visualizer().fireBeam(LeftSide, i, j, 1, 2, 2);
                                    killFighter(m_status[RightSide], j);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// /** \name Beam Weapons. */
// //@{

// /** Recharge beams of one side. Beams recharge randomly up to
//     level 100. */
inline void
game::vcr::classic::HostAlgorithm::rechargeBeams(Status& st)
{
    // ex VcrPlayerTHost::rechargeBeams
    const int mx = st.m_obj.getNumBeams();
    for (int i = 0; i < mx; ++i) {
        register int j = st.m_beamStatus[i];
        if (getRandom_1_100() > 50 && j < 100) {
            st.m_beamStatus[i] = j + st.m_obj.getBeamChargeRate();
            visualizer().updateBeam(st.m_side, i);
        }
    }
}

// /** Fire beam. Hits the enemy with a beam of object from, beam number
//     which. */
inline void
game::vcr::classic::HostAlgorithm::fireBeam(Status& st, Status& opp, int which)
{
    // ex VcrPlayerTHost::fireBeam
    int charge = st.m_beamStatus[which];
    if (const game::spec::Beam* beam = m_shipList.beams().get(st.m_obj.getBeamType())) {
        int da = rdivadd(charge * beam->getDamagePower(), 100, 0);
        int ki = rdivadd(charge * beam->getKillPower(),   100, 0) * st.m_obj.getBeamKillRate();

        hit(opp, da, ki);
        visualizer().fireBeam(st.m_side, -1 - which, -1, 1, da, ki);
    }
    st.m_beamStatus[which] = 0;
    visualizer().updateBeam(st.m_side, which);
}

// /** Fire beams from specified object. Fires all beams that can
//     fire. */
void
game::vcr::classic::HostAlgorithm::fireBeams(Status& st, Status& opp)
{
    // ex VcrPlayerTHost::fireBeams
    for (int i = 0, n = st.m_obj.getNumBeams(); i < n; ++i) {
        int pick = getRandom_1_20();
        if (pick < 7 && st.m_beamStatus[i] > 50) {
            fireBeam(st, opp, i);
        }
    }
}

// /** Fire at fighters. Fires the specified beam at an enemy fighter,
//     if possible. */
void
game::vcr::classic::HostAlgorithm::fireAtFighter(Status& st, Status& opp, int beam)
{
    // ex VcrPlayerTHost::fireAtFighter
    int min_dist = 600;
    int ftr_id   = -1;

    for (int i = 0; i < VCR_MAX_FTRS; ++i) {
        if (opp.m_fighterStatus[i] != FighterIdle) {
            int j = std::abs(opp.m_fighterX[i] - st.m_objectX);
            if (j < min_dist) {
                min_dist = j;
                ftr_id   = i;
            }
        }
    }

    if (ftr_id >= 0) {
        visualizer().fireBeam(st.m_side, -1 - beam, ftr_id, 1, 2, 2 /* FIXME */);
        killFighter(opp, ftr_id);
        st.m_beamStatus[beam] = 0;
        visualizer().updateBeam(st.m_side, beam);
    }
}

// /** Fire beams at fighter. Fire all beams that can fire at enemy
//     fighters. */
void
game::vcr::classic::HostAlgorithm::fireBeamsAtFighter(Status& st, Status& opp)
{
    // ex VcrPlayerTHost::fireBeamsAtFighter
    if (!opp.m_numFightersOut) {
        // enemy has no fighters, so just advance the seed; saves some 2..3% run time
        m_seed = (m_seed + st.m_obj.getNumBeams()) % RANDOM_SIZE;
    } else {
        for (int i = 0, n = st.m_obj.getNumBeams(); i < n; ++i) {
            int pick = getRandom_1_20();
            if (st.m_beamStatus[i] > 40 && pick < 5) {
                fireAtFighter(st, opp, i);
            }
        }
    }
}

// /** Fire torpedo. Launches one torpedo from the specified
//     object. */
inline void
game::vcr::classic::HostAlgorithm::fireTorp(Status& st, Status& opp, int launcher)
{
    // ex VcrPlayerTHost::fireTorp
    register int n = getRandom_1_100();
    if (n >= st.m_obj.getTorpMissRate()) {
        if (const game::spec::TorpedoLauncher* t = m_shipList.launchers().get(st.m_obj.getTorpedoType())) {
            hit(opp, 2*t->getDamagePower(), 2*t->getKillPower());
        }
        m_statistic[st.m_side].handleTorpedoHit();
        visualizer().fireTorpedo(st.m_side, n, launcher);
    } else {
        visualizer().fireTorpedo(st.m_side, -n, launcher);
    }
}

// /** Fire torpedo. Fires all ready torpedoes from the specified
//     object at the enemy. */
void
game::vcr::classic::HostAlgorithm::fireTorpedoes(Status& st, Status& opp)
{
    // ex VcrPlayerTHost::fireTorpedoes
    for (int i = 0, max = st.m_obj.getNumLaunchers(); i < max; ++i) {
        if (st.m_obj.getNumTorpedoes() > 0) {
            int n = getRandom_1_17();
            if (st.m_launcherStatus[i] > 40 || (st.m_launcherStatus[i] > 30 && n < st.m_obj.getTorpedoType())) {
                st.m_obj.addTorpedoes(-1);
                st.m_launcherStatus[i] = 0;
                visualizer().updateLauncher(st.m_side, i);
                fireTorp(st, opp, i);
            }
            st.m_launcherStatus[i] += st.m_obj.getTorpChargeRate();
            visualizer().updateLauncher(st.m_side, i);
        }
    }
}

// /** Charge weapons. If the specified object has full shields,
//     it also starts with fully-charged weapons. */
void
game::vcr::classic::HostAlgorithm::preloadWeapons(Status& st)
{
    // ex VcrPlayerTHost::preloadWeapons
    if (st.m_obj.getShield() == 100) {
        for (int i = 0; i < VCR_MAX_BEAMS; ++i) {
            st.m_launcherStatus[i] = 30;
            st.m_beamStatus[i] = 100;
        }
    }
}

// /** Check one side of VCR. Fixes out-of-range values, and
//     returns true iff there was such a fix needed. */
bool
game::vcr::classic::HostAlgorithm::checkSide(Object& obj)
{
    // ex VcrPlayerTHost::checkVcrSide
    bool err = false;

    if (obj.getOwner() <= 0 || obj.getOwner() > 12 /*FIXME*/) {
        obj.setOwner(12);
    }

    if (obj.getBeamType() != 0 && m_shipList.beams().get(obj.getBeamType()) == 0) {
        obj.setBeamType(0);
        obj.setNumBeams(0);
        err = true;
    }
    if (obj.getTorpedoType() != 0 && m_shipList.launchers().get(obj.getTorpedoType()) == 0) {
        obj.setTorpedoType(0);
        obj.setNumLaunchers(0);
        err = true;
    }
    if (obj.getNumBeams() > VCR_MAX_BEAMS) {
        obj.setNumBeams(VCR_MAX_BEAMS);
        err = true;
    }

    if (obj.getNumLaunchers() > VCR_MAX_TORPS) {
        obj.setNumLaunchers(VCR_MAX_TORPS);
        err = true;
    }

    return err;
}
