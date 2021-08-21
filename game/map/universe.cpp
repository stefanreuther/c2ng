/**
  *  \file game/map/universe.cpp
  */

#include "game/map/universe.hpp"
#include "afl/string/format.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/fleet.hpp"
#include "game/map/objecttype.hpp"
#include "game/map/planet.hpp"
#include "game/map/reverter.hpp"
#include "game/map/ship.hpp"
#include "game/spec/mission.hpp"
#include "util/math.hpp"

namespace {
    /** Format name of a planet. */
    String_t formatPlanetName(const game::map::Planet& pl, afl::string::Translator& tx, int flags)
    {
        return afl::string::Format(((flags & game::map::Universe::NameOrbit) != 0
                                    ? ((flags & game::map::Universe::NameVerbose) != 0
                                       ? tx.translateString("Orbit of %s (Planet #%d)")
                                       : tx.translateString("Orbit of %s (#%d)"))
                                    : ((flags & game::map::Universe::NameVerbose) != 0
                                       ? tx.translateString("%s (Planet #%d)")
                                       : tx.translateString("%s (#%d)"))).c_str(),
                                   pl.getName(tx),
                                   pl.getId());
    }

    /* Format name of a ship.
       This is similar to Ship::getName(LongName), but saves the dependency on InterpreterInterface. */
    String_t formatShipName(const game::map::Ship& sh, afl::string::Translator& tx)
    {
        String_t plainName = sh.getName();
        if (plainName.empty()) {
            return afl::string::Format(tx("Ship #%d"), sh.getId());
        } else {
            return afl::string::Format(tx("Ship #%d: %s"), sh.getId(), plainName);
        }
    }

    /* Postprocess a fleet. */
    void postprocessFleet(game::map::Universe& univ, game::map::Ship& leader,
                          const game::config::HostConfiguration& config,
                          const game::spec::ShipList& shipList)
    {
        // @change PCC2 did more here; in particular, dissolve fleets with invalid Ids.
        // For c2ng, FleetLoader only produces valid, existing fleets.
        // So all that remains is to synchronize missions/waypoints.
        game::map::Fleet(univ, leader).synchronize(config, shipList);
    }

    /* Mark object if it is within a range of coordinates (inclusive). */
    int markObjectIfInRange(game::map::Object& obj, game::map::Point a, game::map::Point b, const game::map::Configuration& config)
    {
        // ex WSelectChartMode::checkObject
        game::map::Point pt;
        if (!obj.getPosition(pt)) {
            return 0;
        }

        // Bounding rectangle in correct size
        const int ax = std::min(a.getX(), b.getX());
        const int bx = std::max(a.getX(), b.getX());
        const int ay = std::min(a.getY(), b.getY());
        const int by = std::max(a.getY(), b.getY());

        // Location. Try to move into bounding rectangle if it's outside.
        if (config.getMode() == game::map::Configuration::Wrapped) {
            if (pt.getX() < ax) {
                pt.addX(config.getSize().getX());
            }
            if (pt.getX() > bx) {
                pt.addX(-config.getSize().getX());
            }
            if (pt.getY() < ay) {
                pt.addY(config.getSize().getY());
            }
            if (pt.getY() > by) {
                pt.addY(-config.getSize().getY());
            }
        }

        // Inside?
        if (pt.getX() >= ax && pt.getX() <= bx && pt.getY() >= ay && pt.getY() <= by) {
            obj.setIsMarked(true);
            return 1;
        } else {
            // Might be circular wrap
            game::map::Point pt1;
            if (config.getMode() == game::map::Configuration::Circular
                && config.getPointAlias(pt, pt1, 1, true)
                && pt1.getX() >= ax && pt1.getX() <= bx && pt1.getY() >= ay && pt1.getY() <= by)
            {
                obj.setIsMarked(true);
                return 1;
            } else {
                return 0;
            }
        }
    }

    /* Mark all objects from an ObjectType if they are in a range of coordinates (inclusive). */
    int markTypeObjectsInRange(game::map::ObjectType& ty, game::map::Point a, game::map::Point b, const game::map::Configuration& config)
    {
        int count = 0;
        for (game::Id_t id = ty.findNextIndex(0); id != 0; id = ty.findNextIndex(id)) {
            if (game::map::Object* obj = ty.getObjectByIndex(id)) {
                count += markObjectIfInRange(*obj, a, b, config);
            }
        }
        return count;
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
      m_reverter(0),
      m_availablePlayers()
{
    // ex GUniverse::GUniverse
    m_minefields.reset(new MinefieldType());
    m_ufos.reset(new UfoType());
    m_explosions.reset(new ExplosionType());
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
     case Reference::Special:
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

game::map::Object*
game::map::Universe::getObject(Reference ref)
{
    return const_cast<Object*>(const_cast<const Universe*>(this)->getObject(ref));
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

void
game::map::Universe::postprocess(PlayerSet_t playingSet, PlayerSet_t availablePlayers, Object::Playability playability,
                                 const game::HostVersion& host, const game::config::HostConfiguration& config,
                                 const int turnNumber,
                                 const game::spec::ShipList& shipList,
                                 afl::string::Translator& tx, afl::sys::LogListener& log)
{
    m_availablePlayers = availablePlayers;

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
    m_ufos->postprocess(turnNumber, m_config, config, tx, log);
    // FIXME: port: setTypePlayability(ty_minefields, playability);
    // FIXME: port: setTypePlayability(ty_ufos, playability);

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
            s->combinedCheck1(*this, availablePlayers, turnNumber);
        }
    }
    for (Id_t i = 1, n = m_planets.size(); i <= n; ++i) {
        if (Planet* p = m_planets.get(i)) {
            p->combinedCheck2(*this, availablePlayers, turnNumber);
        }
    }

    // Fleets
    for (Id_t i = 1, n = m_ships.size(); i <= n; ++i) {
        if (Ship* s = m_ships.get(i)) {
            if (s->isFleetLeader()) {
                postprocessFleet(*this, *s, config, shipList);
            }
        }
    }

    // FIXME: synthesize scores if a score blanker is used
}

bool
game::map::Universe::hasFullData(int playerNr) const
{
    // ex GUniverse::isData
    return m_availablePlayers.contains(playerNr);
}


/*
 *  Location accessors
 */

game::Id_t
game::map::Universe::findPlanetAt(Point pt) const
{
    // ex GUniverse::getPlanetAt, global.pas:PlanetAt
    return AnyPlanetType(const_cast<Universe&>(*this)).findNextObjectAt(m_config.getCanonicalLocation(pt), 0, false);
}

game::Id_t
game::map::Universe::findPlanetAt(Point pt,
                                  bool gravityFlag,
                                  const game::config::HostConfiguration& config,
                                  const HostVersion& host) const
{
    // ex GUniverse::findPlanetAt
    // ex planacc.pas:GravityPlanetAt
    Id_t rv = findPlanetAt(pt);
    if (rv == 0 && gravityFlag) {
        rv = findGravityPlanetAt(pt, config, host);
    }
    return rv;
}

game::Id_t
game::map::Universe::findGravityPlanetAt(Point pt,
                                         const game::config::HostConfiguration& config,
                                         const HostVersion& host) const
{
    // ex GUniverse::getGravityPlanetAt
    // ex planacc.pas:GravityPlanet
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

game::Id_t
game::map::Universe::findFirstShipAt(Point pt) const
{
    // ex GUniverse::getAnyShipAt
    // ex shipacc.pas:ShipAt
    return AnyShipType(const_cast<Universe&>(*this)).findNextObjectAt(m_config.getCanonicalLocation(pt), 0, false);
}

String_t
game::map::Universe::findLocationName(Point pt, int flags,
                                      const game::config::HostConfiguration& config,
                                      const HostVersion& host,
                                      afl::string::Translator& tx) const
{
    // ex GUniverse::getLocationName
    // ex shipacc.pas:LocationStr
    if (Id_t pid = findPlanetAt(pt)) {
        if (const Planet* pl = planets().get(pid)) {
            return formatPlanetName(*pl, tx, flags);
        }
    }

    if ((flags & NameShips) != 0) {
        if (Id_t sid = findFirstShipAt(pt)) {
            if (const Ship* sh = ships().get(sid)) {
                return formatShipName(*sh, tx);
            }
        }
    }

    if ((flags & NameGravity) != 0) {
        if (Id_t pid = findGravityPlanetAt(pt, config, host)) {
            if (const Planet* pl = planets().get(pid)) {
                return afl::string::Format(((flags & NameVerbose) != 0
                                            ? tx.translateString("near %s (Planet #%d)")
                                            : tx.translateString("near %s (#%d)")).c_str(),
                                           pl->getName(tx),
                                           pl->getId());
            }
        }
    }

    if ((flags & NameNoSpace) != 0) {
        return String_t();
    }

    return afl::string::Format((flags & NameVerbose) != 0 ? tx("Deep Space %s") : "%s", pt.toString());
}

game::Id_t
game::map::Universe::findShipTowing(Id_t sid, Id_t after) const
{
    // ex GUniverse::isShipTowed
    AnyShipType ty(const_cast<Universe&>(*this));

    Id_t i = after;
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

game::Id_t
game::map::Universe::findShipCloningAt(Id_t pid, Id_t after) const
{
    // ex GPlanet::findShipCloningHere, GShip::isCloningAt, planacc.pas:FindShipBeingClonedAt, ShipIsCloningAt
    const Planet* p = planets().get(pid);
    Point pt;
    if (p == 0 || !p->getPosition(pt)) {
        return 0;
    }
    
    PlayedShipType& ships = *m_playedShips;
    for (Id_t i = ships.findNextObjectAt(pt, after, false); i != 0; i = ships.findNextObjectAt(pt, i, false)) {
        String_t shipFC;
        const Ship* pShip = ships.getObjectByIndex(i);
        if (pShip != 0 && pShip->getFriendlyCode().get(shipFC) && shipFC == "cln") {
            return i;
        }
    }
    return 0;
}

game::Id_t
game::map::Universe::findControllingPlanetId(const Minefield& mf) const
{
    // ex getAssociatedPlanet, chartdlg.pas:CMinefieldView.SetMinefield
    Id_t pid  = 0;
    int32_t dist = 0;

    Point minePos;
    int mineOwner;
    if (mf.getPosition(minePos) && mf.getOwner(mineOwner)) {
        AnyPlanetType ty(const_cast<Universe&>(*this));
        for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
            if (const Planet* p = m_planets.get(i)) {
                int planetOwner;
                Point planetPos;
                if (p->getPosition(planetPos)) {
                    /* The planet is a possible controlling planet if...
                       - we know it has the same owner as the minefield
                       - we don't know the planet's owner, and don't have full data for the minefield owner's race */
                    bool maybe = (p->getOwner(planetOwner)
                                  ? planetOwner == mineOwner
                                  : !hasFullData(mineOwner));

                    /* Choose closest planet */
                    if (maybe) {
                        int32_t ndist = config().getSquaredDistance(planetPos, minePos);
                        if (pid == 0 || ndist < dist) {
                            pid  = i;
                            dist = ndist;
                        }
                    }
                }
            }
        }
    }

    return pid;
}

int
game::map::Universe::markObjectsInRange(Point a, Point b)
{
    // ex WSelectChartMode::rebuildSelection, part
    AnyShipType ships(*this);
    int numShips = markTypeObjectsInRange(ships, a, b, m_config);

    AnyPlanetType planets(*this);
    int numPlanets = markTypeObjectsInRange(planets, a, b, m_config);

    return numShips + numPlanets;
}

