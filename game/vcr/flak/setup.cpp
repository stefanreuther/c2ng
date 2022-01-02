/**
  *  \file game/vcr/flak/setup.cpp
  *  \brief Class game::vcr::flak::Setup
  */

#include <algorithm>
#include <cmath>
#include "game/vcr/flak/setup.hpp"
#include "afl/base/countof.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/except/filetooshortexception.hpp"
#include "game/playerset.hpp"
#include "game/vcr/flak/configuration.hpp"
#include "game/vcr/flak/environment.hpp"
#include "util/math.hpp"

using afl::except::checkAssertion;

namespace {
    /*
     *  Helper
     */

    void packFleet(const game::vcr::flak::Setup::Fleet& in, game::vcr::flak::structures::Fleet& out)
    {
        out.owner                = static_cast<int16_t>(in.player);
        out.firstShipIndex       = static_cast<int16_t>(in.firstShipIndex);
        out.numShips             = static_cast<int16_t>(in.numShips);
        out.speed                = static_cast<int16_t>(in.speed);
        out.firstAttackListIndex = static_cast<int32_t>(in.firstAttackListIndex);
        out.numAttackListEntries = static_cast<int32_t>(in.numAttackListEntries);
        out.x                    = in.x;
        out.y                    = in.y;
    }
}


/*
 *  game::vcr::flak::Setup::Fleet
 */

game::vcr::flak::Setup::Fleet::Fleet(const structures::Fleet& data)
    : player(data.owner),
      speed(data.speed),
      x(data.x),
      y(data.y),
      firstShipIndex(data.firstShipIndex),
      numShips(data.numShips),
      firstAttackListIndex(data.firstAttackListIndex),
      numAttackListEntries(data.numAttackListEntries)
{
    // ex FlakFleet::FlakFleet
}


/*
 *  game::vcr::flak::Setup
 */

game::vcr::flak::Setup::Setup()
    : m_attackList(),
      m_fleets(),
      m_objects(),
      m_totalTime(0),
      m_seed(0),
      m_position(),
      m_ambientFlags(0)
{
    // ex FlakBattle::FlakBattle
}

game::vcr::flak::Setup::Setup(const Setup& b)
    : m_attackList(b.m_attackList),
      m_totalTime(b.m_totalTime),
      m_seed(b.m_seed),
      m_position(b.m_position),
      m_ambientFlags(b.m_ambientFlags)
{
    // ex FlakBattle::FlakBattle
    // Copy fleets
    for (FleetIndex_t i = 0; i < b.getNumFleets(); ++i) {
        m_fleets.pushBackNew(new Fleet(b.getFleetByIndex(i)));
    }

    // Copy ships
    for (ShipIndex_t i = 0; i < b.getNumShips(); ++i) {
        m_objects.pushBackNew(new Object(b.getShipByIndex(i)));
    }
}

game::vcr::flak::Setup::~Setup()
{ }


/*
 *  Fleets
 */

game::vcr::flak::Setup::FleetIndex_t
game::vcr::flak::Setup::addFleet(int player)
{
    // ex FlakBattle::addFleet
    Fleet& f = *m_fleets.pushBackNew(new Fleet());
    f.player = player;
    f.firstShipIndex = m_objects.size();
    return m_fleets.size()-1;
}

game::vcr::flak::Setup::FleetIndex_t
game::vcr::flak::Setup::getNumFleets() const
{
    // ex FlakBattle::getNumFleets
    return m_fleets.size();
}

void
game::vcr::flak::Setup::startAttackList(FleetIndex_t fleetNr)
{
    Fleet& f = *m_fleets[fleetNr];
    f.firstAttackListIndex = m_attackList.size() / 2;
}

void
game::vcr::flak::Setup::endAttackList(FleetIndex_t fleetNr)
{
    Fleet& f = *m_fleets[fleetNr];
    f.numAttackListEntries = (m_attackList.size() / 2) - f.firstAttackListIndex;
}

void
game::vcr::flak::Setup::addAttackListEntry(ShipIndex_t shipIndex, int16_t ratingBonus)
{
    // ex FlakBattle::addAttackListEntry
    m_attackList.push_back(static_cast<int16_t>(shipIndex));
    m_attackList.push_back(ratingBonus);
}

game::vcr::flak::Setup::Fleet&
game::vcr::flak::Setup::getFleetByIndex(FleetIndex_t number)
{
    // ex FlakBattle::getFleetByNumber
    checkAssertion(number < m_fleets.size(), "range error", "Setup::getFleetByIndex");
    return *m_fleets[number];
}

const game::vcr::flak::Setup::Fleet&
game::vcr::flak::Setup::getFleetByIndex(FleetIndex_t number) const
{
    return const_cast<Setup&>(*this).getFleetByIndex(number);
}

const game::vcr::flak::Setup::AttackList_t&
game::vcr::flak::Setup::getAttackList() const
{
    return m_attackList;
}


/*
 *  Ships
 */

game::vcr::flak::Setup::ShipIndex_t
game::vcr::flak::Setup::addShip(const Object& ship)
{
    // ex FlakBattle::addShip
    checkAssertion(!m_fleets.empty(), "addFleet missing", "Setup::addShip");
    checkAssertion(ship.getOwner() == m_fleets.back()->player, "player mismatch", "Setup::addShip");

    Fleet& theFleet = *m_fleets.back();
    checkAssertion(m_objects.size() == theFleet.firstShipIndex + theFleet.numShips, "index mismatch", "Setup::addShip");
    ++theFleet.numShips;

    m_objects.pushBackNew(new Object(ship));
    return m_objects.size()-1;
}

game::vcr::flak::Setup::ShipIndex_t
game::vcr::flak::Setup::getNumShips() const
{
    return m_objects.size();
}

game::vcr::flak::Object&
game::vcr::flak::Setup::getShipByIndex(size_t number)
{
    // ex FlakBattle::getShipByNumber
    checkAssertion(number < m_objects.size(), "range error", "Setup::getShipByIndex");
    return *m_objects[number];
}

const game::vcr::flak::Object&
game::vcr::flak::Setup::getShipByIndex(size_t number) const
{
    return const_cast<Setup&>(*this).getShipByIndex(number);
}


/*
 *  Other attributes
 */

int32_t
game::vcr::flak::Setup::getTotalTime() const
{
    // ex FlakBattle::getTime (sort-of)
    return m_totalTime;
}

void
game::vcr::flak::Setup::setTotalTime(int32_t time)
{
    m_totalTime = time;
}

uint32_t
game::vcr::flak::Setup::getSeed() const
{
    // ex FlakBattle::getOrigSeed (sort-of)
    return m_seed;
}

void
game::vcr::flak::Setup::setSeed(uint32_t seed)
{
    // ex FlakBattle::setOrigSeed (sort-of), FlakBattle::setInitialSeed
    m_seed = seed;
}

bool
game::vcr::flak::Setup::getPosition(game::map::Point& pos) const
{
    if (m_position != game::map::Point()) {
        pos = m_position;
        return true;
    } else {
        return false;
    }
}

void
game::vcr::flak::Setup::setPosition(game::map::Point pos)
{
    m_position = pos;
}

int32_t
game::vcr::flak::Setup::getAmbientFlags() const
{
    return m_ambientFlags;
}

void
game::vcr::flak::Setup::setAmbientFlags(int32_t flags)
{
    m_ambientFlags = flags;
}


/*
 *  I/O
 */

void
game::vcr::flak::Setup::save(afl::base::GrowableBytes_t& s, afl::charset::Charset& charset) const
{
    // ex FlakBattle::saveToStream
    structures::Battle data;
    data.x                    = static_cast<int16_t>(m_position.getX());
    data.y                    = static_cast<int16_t>(m_position.getY());
    data.seed                 = m_seed;
    data.num_fleets           = static_cast<int32_t>(m_fleets.size());
    data.num_ships            = static_cast<int32_t>(m_objects.size());
    data.num_att_list_entries = static_cast<int32_t>(m_attackList.size() / 2);
    data.total_time           = m_totalTime;
    data.ambient_flags        = m_ambientFlags;
    data.fleet_entry_size     = sizeof(structures::Fleet);
    data.fleet_ptr            = sizeof(structures::Battle);
    data.ship_entry_size      = sizeof(structures::Ship);
    data.ship_ptr             = static_cast<int32_t>(data.fleet_ptr + sizeof(structures::Fleet) * m_fleets.size());
    data.att_list_entry_size  = 4;
    data.att_list_ptr         = static_cast<int32_t>(data.ship_ptr + sizeof(structures::Ship) * m_objects.size());
    data.this_size            = data.att_list_ptr + 4 * data.num_att_list_entries;

    s.append(afl::base::fromObject(data));
    for (FleetIndex_t i = 0; i < m_fleets.size(); ++i) {
        structures::Fleet fleet;
        packFleet(*m_fleets[i], fleet);
        s.append(afl::base::fromObject(fleet));
    }
    for (ShipIndex_t i = 0; i < m_objects.size(); ++i) {
        structures::Ship ship;
        m_objects[i]->pack(ship, charset);
        s.append(afl::base::fromObject(ship));
    }

    /* attack list */
    for (std::size_t i = 0; i < m_attackList.size(); ++i) {
        structures::Int16_t value;
        value = m_attackList[i];
        s.append(value.m_bytes);
    }
}

void
game::vcr::flak::Setup::load(String_t name, afl::base::ConstBytes_t s, afl::charset::Charset& charset, afl::string::Translator& tx)
{
    // ex FlakBattle::loadFromStream
    // FIXME: validation needs to be tightened here!

    // Clear everything
    m_objects.clear();
    m_fleets.clear();
    m_attackList.clear();

    // Read/validate header
    structures::Battle header;
    if (s.size() < sizeof(header)) {
        throw afl::except::FileTooShortException(name);
    }
    afl::base::fromObject(header).copyFrom(s);

    if (header.fleet_entry_size != sizeof(structures::Fleet) || header.ship_entry_size != sizeof(structures::Ship) || header.att_list_entry_size != 2*sizeof(int16_t)) {
        throw afl::except::FileFormatException(name, tx("Unsupported file format"));
    }
    if (!header.num_fleets || !header.num_ships || !header.num_att_list_entries) {
        throw afl::except::FileFormatException(name, tx("Invalid file format (object count is zero)"));
    }

    // Set singular attributes
    setSeed(header.seed);
    setAmbientFlags(header.ambient_flags);
    setTotalTime(header.total_time);
    setPosition(game::map::Point(header.x, header.y));

    // Load fleets
    size_t ptr = header.fleet_ptr;
    int32_t ship_counter = 0;
    for (int32_t i = 0; i < header.num_fleets; ++i) {
        structures::Fleet f;
        afl::base::fromObject(f).copyFrom(s.subrange(ptr));

        if (f.firstShipIndex != ship_counter || f.firstShipIndex + f.numShips > header.num_ships) {
            throw afl::except::FileFormatException(name, tx("Invalid file format (inconsistent ship numbering)"));
        }
        if (f.firstAttackListIndex < 0 || f.firstAttackListIndex > header.num_att_list_entries
            || f.numAttackListEntries < 0 || f.firstAttackListIndex + f.numAttackListEntries > header.num_att_list_entries)
        {
            throw afl::except::FileFormatException(name, tx("Invalid file format (invalid attack list pointer)"));
        }

        ship_counter += f.numShips;
        m_fleets.pushBackNew(new Fleet(f));
        ptr += header.fleet_entry_size;
    }
    if (ship_counter != header.num_ships) {
        throw afl::except::FileFormatException(name, tx("Invalid file format (inconsistent ship numbering)"));
    }

    // Load ships
    ptr = header.ship_ptr;
    size_t fleet_counter = 0;
    for (int32_t i = 0; i < header.num_ships; ++i) {
        structures::Ship sh;
        afl::base::fromObject(sh).copyFrom(s.subrange(ptr));

        if (sh.owner != m_fleets[fleet_counter]->player) {
            throw afl::except::FileFormatException(name, tx("Invalid file format (invalid owner information)"));
        }

        m_objects.pushBackNew(new Object(sh, charset));
        if (m_objects.size() >= m_fleets[fleet_counter]->firstShipIndex + m_fleets[fleet_counter]->numShips) {
            ++fleet_counter;
        }

        ptr += header.ship_entry_size;
    }

    // attack list
    ptr = header.att_list_ptr;
    for (int32_t i = 0; i < header.num_att_list_entries; ++i) {
        structures::Int16_t ele[2];
        afl::base::fromObject(ele).copyFrom(s.subrange(ptr));
        m_attackList.push_back(ele[0]);
        m_attackList.push_back(ele[1]);
        ptr += header.att_list_entry_size;
    }
}


/*
 *  Setup
 */

void
game::vcr::flak::Setup::initAfterSetup(const Configuration& config, const Environment& env, util::RandomNumberGenerator& rng)
{
    // ex FlakBattle::initAfterSetup
    removePassiveObjects();
    computeFleetSpeeds(env);
    computeInitialPositions(config, rng);
    if (config.CompensationAdjust) {
        adjustStrengths(config.CompensationAdjust, config);
    }
}


/** Remove all passive objects.
    Units which are not attackable should not appear in the data to not leak too much information. */
void
game::vcr::flak::Setup::removePassiveObjects()
{
    // ex FlakBattle::removePassiveObjects, flak.pas:RemovePassiveObjects
    // temporary array
    std::vector<int> new_ids(m_objects.size());

    // first, mark all referenced ships. We use the fact that a ship
    // which is in no attack list can itself not attack anyone and
    // therefore is useless. After this loop, all units which are used
    // have a 1 in new_ids[].
    for (FleetIndex_t i = 0; i < m_fleets.size(); ++i) {
        Fleet& f = *m_fleets[i];
        for (size_t n = 0; n < f.numAttackListEntries; ++n) {
            if (m_attackList[2 * (f.firstAttackListIndex + n) + 1] > 0) {
                new_ids[m_attackList[2 * (f.firstAttackListIndex + n)]] = 1;
            }
        }
    }

    // now, recompute stuff
    ShipIndex_t id = 0;
    for (ShipIndex_t i = 0; i < m_objects.size(); ++i) {
        if (new_ids[i] == 0) {
            /* ship does not appear in lists */
            new_ids[i] = -1;
        } else {
            new_ids[i] = static_cast<int>(id);
            m_objects.swapElements(i, id);
            ++id;
        }
    }

    m_objects.resize(id);

    /* remap data: attack lists */
    //     for (std::size_t i = 0; i < m_attackList.size(); i += 2) {
    //         ASSERT(new_ids[m_attackList[i]] >= 0);
    //         m_attackList[i] = new_ids[m_attackList[i]];
    //     }
    std::vector<int16_t> new_att_list;
    for (std::size_t i = 0; i < m_fleets.size(); ++i) {
        Fleet& me = *m_fleets[i];
        size_t new_start = static_cast<uint32_t>(new_att_list.size() / 2);
        size_t new_length = 0;
        for (size_t n = 0; n < me.numAttackListEntries; ++n) {
            int x = new_ids[m_attackList[2 * (me.firstAttackListIndex + n)]];
            if (x >= 0) {
                new_att_list.push_back(static_cast<int16_t>(x));
                new_att_list.push_back(static_cast<int16_t>(m_attackList[2 * (me.firstAttackListIndex + n) + 1]));
                ++new_length;
            }
        }
        me.firstAttackListIndex = new_start;
        me.numAttackListEntries = new_length;
    }
    new_att_list.swap(m_attackList);

    /* remap data: fleet pointers */
    size_t fleet_id = 0;
    for (size_t i = 0; i < m_fleets.size(); ++i) {
        size_t num = 0;
        size_t ship_nr = m_fleets[i]->firstShipIndex;
        for (size_t x = 0; x < size_t(m_fleets[i]->numShips); ++x) {
            if (new_ids[ship_nr + x] >= 0) {
                ++num;
            }
        }
        if (num != 0) {
            /* fleet still there, figure out new first ship. */
            while (new_ids[ship_nr] < 0) {
                ++ship_nr;
            }
            m_fleets[i]->firstShipIndex = new_ids[ship_nr];
            m_fleets[i]->numShips = num;
            m_fleets.swapElements(fleet_id, i);
            ++fleet_id;
        }
    }
    m_fleets.resize(fleet_id);
}

/** Compute speeds of all fleets.
    This function needs access to the host-side configuration (pconfig). */
void
game::vcr::flak::Setup::computeFleetSpeeds(const Environment& env)
{
    // ex FlakBattle::computeFleetSpeeds, flak.pas:ComputeFleetSpeeds
    for (FleetIndex_t i = 0; i < m_fleets.size(); ++i) {
        int speed = 0;
        for (size_t j = 0; j < m_fleets[i]->numShips; ++j) {
            int this_speed;
            const Object& this_ship = *m_objects[j + m_fleets[i]->firstShipIndex];
            if (this_ship.isPlanet()) {
                this_speed = 0;
            } else {
                this_speed = env.getConfiguration(Environment::ShipMovementSpeed, this_ship.getOwner());
            }

            if (j == 0 || this_speed < speed) {
                speed = this_speed;
            }
        }
        m_fleets[i]->speed = speed;
    }
}

/** Compute initial fleet positions.
    This needs access to the host-side configuration (FLAK config). */
void
game::vcr::flak::Setup::computeInitialPositions(const Configuration& config, util::RandomNumberGenerator& rng)
{
    // ex FlakBattle::computeInitialPositions, flak.pas:ComputeInitialPositions
    /* do we have a planet? */
    int planet_owner = 0;
    int num_players = 0;
    PlayerSet_t players;
    for (ShipIndex_t i = 0; i < m_objects.size(); ++i) {
        checkAssertion(m_objects[i]->getOwner() > 0 && m_objects[i]->getOwner() <= FLAK_NUM_OWNERS, "player range", "Setup::computeInitialPositions");
        if (m_objects[i]->isPlanet()) {
            checkAssertion(!planet_owner, "multiple planets", "Setup::computeInitialPositions");
            planet_owner = m_objects[i]->getOwner();
        }
        if (i == 0 || m_objects[i]->getOwner() != m_objects[i-1]->getOwner()) {
            checkAssertion(!players.contains(m_objects[i]->getOwner()), "discontinuity", "Setup::computeInitialPositions");
            ++num_players;
            players |= m_objects[i]->getOwner();
        }
    }

    /* shuffle players */
    int counter;
    int offs = config.StartingDistancePerPlayer * num_players;
    int player_list[FLAK_NUM_OWNERS];

    counter = 0;
    for (int i = 1; i <= FLAK_NUM_OWNERS; ++i) {
        if (players.contains(i)) {
            player_list[counter++] = i;
        }
    }
    checkAssertion(counter == num_players, "player count", "Setup::computeInitialPositions");
    for (int i = num_players-1; i > 0; --i) {
        std::swap(player_list[i], player_list[rng(static_cast<uint16_t>(i+1))]);
    }

    /* assign positions */
    counter = 0;
    for (int index = 0; index < num_players; ++index) {
        const int player = player_list[index];
        if (planet_owner) {
            if (player == planet_owner) {
                assignInitialPositions(player, 0, config.StartingDistancePlanet + offs, config);
            } else {
                assignInitialPositions(player, (util::PI/2) + (2*counter+1) * (util::PI/2) / (num_players-1),
                                       config.StartingDistanceShip + offs, config);
                ++counter;
            }
        } else {
            assignInitialPositions(player, (util::PI/2) + (2*counter+1)*util::PI / num_players,
                                   config.StartingDistanceShip + offs, config);
            ++counter;
        }
    }
}

/** Assign initial positions for a player.
    \param player   player number
    \param angle    angle (in radians) of first fleet
    \param dist     distance from origin, for first fleet
    \param config   FLAK configuration */
void
game::vcr::flak::Setup::assignInitialPositions(const int player, double angle, int32_t dist, const Configuration& config)
{
    // ex FlakBattle::assignInitialPositions, flak.pas:AssignInitialPositions
    int offset = config.StartingDistancePerFleet;
    for (FleetIndex_t i = 0; i < m_fleets.size(); ++i) {
        Fleet& this_fleet = *m_fleets[i];
        if (this_fleet.player != player) {
            continue;
        }
        if (this_fleet.speed > offset) {
            offset = this_fleet.speed;
        }

        this_fleet.x = util::roundToInt(std::cos(angle) * dist);
        this_fleet.y = util::roundToInt(std::sin(angle) * dist);
        dist += offset;
        angle += util::PI/180.0;  /* 1 degree */
    }
}

/** Adjust compensation. */
void
game::vcr::flak::Setup::adjustStrengths(int adj_to, const Configuration& config)
{
    // ex FlakBattle::adjustStrengths, flak.pas:AdjustStrengths
    int32_t total_strength = 0;
    for (ShipIndex_t i = 0; i < m_objects.size(); ++i) {
        total_strength += m_objects[i]->getCompensation();
    }

    int32_t target_strength = int32_t(m_objects.size()) * adj_to;
    if (total_strength != 0 && total_strength < target_strength) {
        for (ShipIndex_t i = 0; i < m_objects.size(); ++i) {
            int32_t new_val = m_objects[i]->getCompensation() * target_strength / total_strength;
            if (new_val > config.CompensationLimit) {
                new_val = config.CompensationLimit;
            }
            m_objects[i]->setCompensation(new_val);
        }
    }
}
