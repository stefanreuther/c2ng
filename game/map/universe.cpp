/**
  *  \file game/map/universe.cpp
  */

#include "game/map/universe.hpp"
#include "game/map/planet.hpp"
#include "game/map/objecttype.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/ship.hpp"
#include "game/map/reverter.hpp"
#include "afl/string/format.hpp"
#include "game/map/anyshiptype.hpp"
#include "util/math.hpp"
#include "game/spec/mission.hpp"

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
      m_ufos(),
      m_explosions(),
      m_drawings(),
      m_universeChanged(false),
      m_playedShips(),
      m_playedPlanets(),
      m_playedBases(),
      m_fleets(),
      m_ionStormType(),
      m_reverter(0)
{
    // ex GUniverse::GUniverse
    m_minefields.reset(new MinefieldType(*this));
    m_ufos.reset(new UfoType(*this));
    m_explosions.reset(new ExplosionType(*this));
    m_playedShips.reset(new PlayedShipType(*this));
    m_playedPlanets.reset(new PlayedPlanetType(*this));
    m_playedBases.reset(new PlayedBaseType(*this));
    m_fleets.reset(new FleetType(*this));
    m_ionStormType.reset(new IonStormType(*this));

    m_drawings.sig_change.add(this, &Universe::markChanged);

    // Observe all set changes
    playedShips().sig_setChange.add(this, &Universe::markChanged);
    playedPlanets().sig_setChange.add(this, &Universe::markChanged);
    ionStormType().sig_setChange.add(this, &Universe::markChanged);
    minefields().sig_setChange.add(this, &Universe::markChanged);
    ufos().sig_setChange.add(this, &Universe::markChanged);
    explosions().sig_setChange.add(this, &Universe::markChanged);
}

game::map::Universe::~Universe()
{ }

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

const game::map::PlayedShipType&
game::map::Universe::playedShips() const
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

game::map::FleetType&
game::map::Universe::fleets()
{
    return *m_fleets;
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

game::map::UfoType&
game::map::Universe::ufos()
{
    return *m_ufos;
}

const game::map::UfoType&
game::map::Universe::ufos() const
{
    return *m_ufos;
}

game::map::ExplosionType&
game::map::Universe::explosions()
{
    return *m_explosions;
}

const game::map::ExplosionType&
game::map::Universe::explosions() const
{
    return *m_explosions;
}

game::map::DrawingContainer&
game::map::Universe::drawings()
{
    return m_drawings;
}

const game::map::DrawingContainer&
game::map::Universe::drawings() const
{
    return m_drawings;
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
game::map::Universe::setNewReverter(Reverter* p)
{
    m_reverter.reset(p);
}

game::map::Reverter*
game::map::Universe::getReverter() const
{
    return m_reverter.get();
}

const game::map::Object*
game::map::Universe::getObject(Reference ref) const
{
    switch (ref.getType()) {
     case Reference::Null:
     case Reference::Player:
     case Reference::MapLocation:
        return 0;

     case Reference::Ship:
        return ships().get(ref.getId());
     case Reference::Planet:
     case Reference::Starbase:
        return planets().get(ref.getId());
     case Reference::Storm:
        return ionStorms().get(ref.getId());
     case Reference::Minefield:
        return minefields().get(ref.getId());
     case Reference::Ufo: {
        UfoType& ty = const_cast<UfoType&>(ufos());
        return ty.getObjectByIndex(ty.findIndexForId(ref.getId()));
     }

     case Reference::Hull:
     case Reference::Engine:
     case Reference::Beam:
     case Reference::Torpedo:
        return 0;
    }
    return 0;
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
    changed |= m_ufos->notifyObjectListeners();
    changed |= m_explosions->notifyObjectListeners();

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
                                 const int turnNumber,
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
    m_minefields->internalCheck(turnNumber, host, config);
    m_drawings.eraseExpiredDrawings(turnNumber);
    m_ufos->postprocess(turnNumber);
// FIXME: port
//     setTypePlayability(ty_minefields, playability);
//     setTypePlayability(ty_ufos, playability);

    // Signal sets
    m_playedShips->sig_setChange.raise(0);
    m_playedPlanets->sig_setChange.raise(0);
    m_playedBases->sig_setChange.raise(0);
    m_fleets->sig_setChange.raise(0);
    m_ionStormType->sig_setChange.raise(0);
    m_minefields->sig_setChange.raise(0);
    m_ufos->sig_setChange.raise(0);
    m_explosions->sig_setChange.raise(0);

    // Combined checks
    for (Id_t i = 1, n = m_ships.size(); i <= n; ++i) {
        if (Ship* s = m_ships.get(i)) {
            s->combinedCheck1(*this, turnNumber);
        }
    }
    for (Id_t i = 1, n = m_planets.size(); i <= n; ++i) {
        if (Planet* p = m_planets.get(i)) {
            p->combinedCheck2(*this, availablePlayers, turnNumber);
        }
    }

    // FIXME: synthesize scores if a score blanker is used
}


// /** Get planet at location.
//     \param pt Location, need not be normalized
//     \return Id of planet, or zero if none */
game::Id_t
game::map::Universe::getPlanetAt(Point pt) const
{
    // ex GUniverse::getPlanetAt
    return AnyPlanetType(const_cast<Universe&>(*this)).findFirstObjectAt(m_config.getCanonicalLocation(pt));
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
                                 const HostVersion& host) const
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
                                        const HostVersion& host) const
{
    // ex GUniverse::getGravityPlanetAt
    /* easy case */
    if (!config[config.AllowGravityWells]()) {
        return 0;
    }

    AnyPlanetType ty(const_cast<Universe&>(*this));
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

// /** Check whether a ship is being towed.
//     \param sid Ship Id */
game::Id_t
game::map::Universe::findShipTowing(int sid, int after) const
{
    // ex GUniverse::isShipTowed
    AnyShipType ty(const_cast<Universe&>(*this));

    int i = after;
    while ((i = ty.findNextIndex(i)) != 0) {
        const Ship* sh = ty.getObjectByIndex(i);
        if (sh != 0
            && sh->isPlayable(Object::ReadOnly)
            && sh->getMission().orElse(0) == game::spec::Mission::msn_Tow
            && sh->getMissionParameter(TowParameter).orElse(0) == sid)
        {
            return i;
        }
    }
    return 0;
}

