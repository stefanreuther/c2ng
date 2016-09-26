/**
  *  \file game/map/universe.cpp
  */

#include "game/map/universe.hpp"
#include "game/map/planet.hpp"
#include "game/map/objecttype.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/ship.hpp"
#include "afl/string/format.hpp"
#include "game/map/anyshiptype.hpp"
#include "util/math.hpp"

namespace {
    /** Format name of a planet. */
    String_t formatPlanetName(const game::map::Planet& pl, afl::string::Translator& tx, game::InterpreterInterface& iface, int flags)
    {
        return afl::string::Format(((flags & game::map::Universe::NameOrbit) != 0
                                    ? ((flags & game::map::Universe::NameVerbose) != 0
                                       ? tx.translateString("Orbit of %s (Planet #%d)")
                                       : tx.translateString("Orbit of %s (#%d)"))
                                    : ((flags & game::map::Universe::NameVerbose) != 0
                                       ? tx.translateString("%s (Planet #%d)")
                                       : tx.translateString("%s (#%d)"))).c_str(),
                                   pl.getName(game::map::Planet::PlainName, tx, iface),
                                   pl.getId());
    }
}



game::map::Universe::Universe()
    : sig_preUpdate(),
      sig_universeChange(),
      m_config(),
      m_planets(),
      m_ionStorms(),
      m_minefields(),
      m_universeChanged(false),
      m_turnNumber(0),
      m_timestamp(),
      m_playedShips(),
      m_playedPlanets(),
      m_playedBases(),
      m_ionStormType()
{
    m_minefields.reset(new MinefieldType(*this));
    m_playedShips.reset(new PlayedShipType(*this));
    m_playedPlanets.reset(new PlayedPlanetType(*this));
    m_playedBases.reset(new PlayedBaseType(*this));
    m_ionStormType.reset(new IonStormType(*this));
// /** Create blank universe. */
// GUniverse::GUniverse()
//       ty_any_ships(*this),
//       ty_history_ships(*this),
//       ty_any_planets(*this),
//       ty_fleets(*this),
//       ty_ufos(*this),
//       ty_minefields(*this),
// {
//     init();
// }

}

game::map::Universe::~Universe()
{ }


int
game::map::Universe::getTurnNumber() const
{
    return m_turnNumber;
}

void
game::map::Universe::setTurnNumber(int nr)
{
    m_turnNumber = nr;
}

game::Timestamp
game::map::Universe::getTimestamp() const
{
    return m_timestamp;
}

void
game::map::Universe::setTimestamp(const Timestamp& ts)
{
    m_timestamp = ts;
}

game::map::ObjectVector<game::map::Ship>&
game::map::Universe::ships()
{
    // ex GUniverse::getShip, GUniverse::isValidShipId (sort-of)
    return m_ships;
}

const game::map::ObjectVector<game::map::Ship>&
game::map::Universe::ships() const
{
    return m_ships;
}

game::map::PlayedShipType&
game::map::Universe::playedShips()
{
    return *m_playedShips;
}

game::map::ObjectVector<game::map::Planet>&
game::map::Universe::planets()
{
    // ex GUniverse::getPlanet, GUniverse::isValidPlanetId (sort-of)
    return m_planets;
}
    
const game::map::ObjectVector<game::map::Planet>&
game::map::Universe::planets() const
{
    return m_planets;
}

game::map::PlayedPlanetType&
game::map::Universe::playedPlanets()
{
    return *m_playedPlanets;
}

game::map::PlayedBaseType&
game::map::Universe::playedBases()
{
    return *m_playedBases;
}

game::map::ObjectVector<game::map::IonStorm>&
game::map::Universe::ionStorms()
{
    // ex GUniverse::isValidIonStormId, GUniverse::getIonStorm (sort-of)
    return m_ionStorms;
}

const game::map::ObjectVector<game::map::IonStorm>&
game::map::Universe::ionStorms() const
{
    return m_ionStorms;
}

game::map::IonStormType&
game::map::Universe::ionStormType()
{
    return *m_ionStormType;
}

game::map::MinefieldType&
game::map::Universe::minefields()
{
    return *m_minefields;
}

const game::map::MinefieldType&
game::map::Universe::minefields() const
{
    return *m_minefields;
}


game::map::Configuration&
game::map::Universe::config()
{
    return m_config;
}

const game::map::Configuration&
game::map::Universe::config() const
{
    return m_config;
}


void
game::map::Universe::notifyListeners()
{
    // ex GUniverse::doScreenUpdates
    bool changed = false;

    /* Tell everyone we're going to do updates */
    sig_preUpdate.raise();

    /* Update individual objects */
    // changed |= updateType(ty_history_ships);
    changed |= AnyShipType(*this).notifyObjectListeners();
    changed |= AnyPlanetType(*this).notifyObjectListeners();
    changed |= m_ionStormType->notifyObjectListeners();
    changed |= m_minefields->notifyObjectListeners();
    // changed |= updateType(ty_ufos);

    /* Tell everyone we did updates */
    if (changed || m_universeChanged) {
        sig_universeChange.raise();
    }
    m_universeChanged = false;
}

void
game::map::Universe::markChanged()
{
    m_universeChanged = true;
}

// /** Postprocess universe. Call this to make sure structural changes propagate.
//     \param playingSet Set of players we play
//     \param dataSet Set of players we have data for
//     \param playability Playability to set for things we "play" */
void
game::map::Universe::postprocess(PlayerSet_t playingSet, PlayerSet_t availablePlayers, Object::Playability playability,
                                 const game::HostVersion& host, const game::config::HostConfiguration& config,
                                 afl::string::Translator& tx, afl::sys::LogListener& log)
{
//     this->playing_set = playing_set;
//     this->data_set = data_set;

    // Internal check for ships and planets
    for (Id_t i = 1, n = m_planets.size(); i <= n; ++i) {
        if (Planet* p = m_planets.get(i)) {
            p->internalCheck(m_config, tx, log);

            int owner;
            if (p->getOwner(owner)) {
                p->setPlayability(p->isVisible() && p->hasFullPlanetData() && owner != 0
                                  ? (playingSet.contains(owner)
                                     ? playability
                                     : Object::ReadOnly)
                                  : Object::NotPlayable);
            } else {
                p->setPlayability(Object::NotPlayable);
            }
        }
    }

    for (Id_t i = 1, n = m_ships.size(); i <= n; ++i) {
        if (Ship* s = m_ships.get(i)) {
            s->internalCheck();

            int owner;
            if (s->getOwner(owner)) {
                s->setPlayability(s->isVisible() && s->hasFullShipData()
                                  ? (playingSet.contains(owner)
                                     ? playability
                                     : Object::ReadOnly)
                                  : Object::NotPlayable);
            } else {
                s->setPlayability(Object::NotPlayable);
            }
        }
    }

    // Internal checks for others
    m_minefields->internalCheck(m_turnNumber, host, config);
// FIXME: port
//     drawings.eraseExpiredDrawings(getTurnNumber());
//     ty_ufos.postprocess();
//     setTypePlayability(ty_minefields, playability);
//     setTypePlayability(ty_ufos, playability);

    // Signal sets
    m_playedShips->sig_setChange.raise(0);
    m_playedPlanets->sig_setChange.raise(0);
    m_playedBases->sig_setChange.raise(0);
    m_ionStormType->sig_setChange.raise(0);

    // Combined checks
    for (Id_t i = 1, n = m_ships.size(); i <= n; ++i) {
        if (Ship* s = m_ships.get(i)) {
            s->combinedCheck1(*this);
        }
    }
    for (Id_t i = 1, n = m_planets.size(); i <= n; ++i) {
        if (Planet* p = m_planets.get(i)) {
            p->combinedCheck2(*this, availablePlayers);
        }
    }

    // FIXME: synthesize scores if a score blanker is used
}


// /** Get planet at location.
//     \param pt Location, need not be normalized
//     \return Id of planet, or zero if none */
game::Id_t
game::map::Universe::getPlanetAt(Point pt)
{
    // ex GUniverse::getPlanetAt
    return AnyPlanetType(*this).findFirstObjectAt(m_config.getCanonicalLocation(pt));
}

// /** Get planet at location, with warp wells. If there is no planet at
//     the location, look whether the point is in the warp well of one.
//     \param pt Location, need not be normalized
//     \param gravity true to look into warp well. If false, function
//                    behaves exactly like getPlanetAt(int).
//     \return Id of planet, or zero if none */
game::Id_t
game::map::Universe::getPlanetAt(Point pt,
                                 bool gravityFlag,
                                 const game::config::HostConfiguration& config,
                                 const HostVersion& host)
{
    // ex GUniverse::getPlanetAt
    Id_t rv = getPlanetAt(pt);
    if (rv == 0 && gravityFlag) {
        rv = getGravityPlanetAt(pt, config, host);
    }
    return rv;
}


// /** Get planet from warp well location.
//     \param pt Location
//     \pre getPlanetAt(pt) == 0
//     \return Id of planet if pt is in its warp wells, 0 otherwise */
game::Id_t
game::map::Universe::getGravityPlanetAt(Point pt,
                                        const game::config::HostConfiguration& config,
                                        const HostVersion& host)
{
    // ex GUniverse::getGravityPlanetAt
    /* easy case */
    if (!config[config.AllowGravityWells]()) {
        return 0;
    }

    AnyPlanetType ty(*this);
    switch (host.getKind()) {
     case HostVersion::Unknown:
     case HostVersion::PHost: {
        /* PHost gravity wells */
        int sqs = util::squareInteger(config[config.GravityWellRange]());
        for (Id_t i = ty.getPreviousIndex(0); i != 0; i = ty.getPreviousIndex(i)) {
            if (const Planet* p = ty.getObjectByIndex(i)) {
                Point pos;
                if (p->getPosition(pos)) {
                    if (config[config.RoundGravityWells]()) {
                        if (m_config.getSquaredDistance(pos, pt) <= sqs)
                            return i;
                    } else {
                        Point p2 = m_config.getSimpleNearestAlias(pos, pt);
                        if (util::squareInteger(p2.getX() - pt.getX()) <= sqs && util::squareInteger(p2.getY() - pt.getY()) <= sqs)
                            return i;
                    }
                }
            }
        }
        return 0;
     }

     case HostVersion::SRace:
     case HostVersion::Host:
     case HostVersion::NuHost: {     // FIXME: does this go here?
        /* THost gravity wells: round, 3 ly, not wrapped, "cumulative" */
        Id_t pid = 0;
        for (Id_t i = ty.getNextIndex(0); i != 0; i = ty.getNextIndex(i)) {
            if (const Planet* p = ty.getObjectByIndex(i)) {
                Point pos;
                if (p->getPosition(pos)) {
                    if (m_config.getSquaredDistance(pos, pt) <= 9) {
                        pt = pos;  // (!)
                        pid = i;
                    }
                }
            }
        }
        return pid;
     }
    }
    return 0;
}

/** Get ship at position. Any race does. This is mainly used for naming
    locations, and for things like to tell whether a "L" command would succeed.
    \param pt position to check
    \returns Id number of a ship at position \c pt, or zero if none. */
game::Id_t
game::map::Universe::getAnyShipAt(Point pt)
{
    // ex GUniverse::getAnyShipAt
    return AnyShipType(*this).findFirstObjectAt(m_config.getCanonicalLocation(pt));
}

// /** Get name of a location in human-readable form.
//     \param pt     location
//     \param flags  details of requested string
//     - locs_Ships: also use ship names as location names, not just planets
//     - locs_WW: return "near PLANETNAME" if applicable
//     - locs_Orbit: say "Orbit of PLANET" if applicable
//     - locs_Verbose: be more verbose */
String_t
game::map::Universe::getLocationName(Point pt, int flags,
                                     const game::config::HostConfiguration& config,
                                     const HostVersion& host,
                                     afl::string::Translator& tx,
                                     InterpreterInterface& iface)
{
    // ex GUniverse::getLocationName
    if (Id_t pid = getPlanetAt(pt)) {
        if (const Planet* pl = planets().get(pid)) {
            return formatPlanetName(*pl, tx, iface, flags);
        }
    }

    if ((flags & NameShips) != 0) {
        if (Id_t sid = getAnyShipAt(pt)) {
            if (const Ship* sh = ships().get(sid)) {
                return sh->getName(Ship::LongName, tx, iface);
            }
        }
    }

    if ((flags & NameGravity) != 0) {
        if (Id_t pid = getGravityPlanetAt(pt, config, host)) {
            if (const Planet* pl = planets().get(pid)) {
                return afl::string::Format(((flags & NameVerbose) != 0
                                            ? tx.translateString("near %s (Planet #%d)")
                                            : tx.translateString("near %s (#%d)")).c_str(),
                                           pl->getName(Planet::PlainName, tx, iface),
                                           pl->getId());
            }
        }
    }

    if ((flags & NameNoSpace) != 0) {
        return String_t();
    }

    return afl::string::Format((flags & NameVerbose) != 0 ? tx.translateString("Deep Space (%d,%d)").c_str() : "(%d,%d)", pt.getX(), pt.getY());
}
