/**
  *  \file game/map/locker.cpp
  *  \brief Class game::map::Locker
  */

#include <climits>
#include "game/map/locker.hpp"
#include "game/config/bitsetvalueparser.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/configuration.hpp"
#include "game/map/mapobject.hpp"
#include "game/map/universe.hpp"

namespace {
    game::config::BitsetValueParser g_lockOptionParser("planet,ship,ufo,marker,minefield");
}

const game::map::LockOptionDescriptor_t game::map::Lock_Left = {
    "Lock.Left",
    &g_lockOptionParser,
    MatchPlanets | MatchMinefields | MatchUfos
};

const game::map::LockOptionDescriptor_t game::map::Lock_Right = {
    "Lock.Right",
    &g_lockOptionParser,
    MatchShips | MatchDrawings
};


game::map::Locker::Locker(Point target, const Configuration& config)
    : m_target(target),
      m_min(INT_MIN, INT_MIN),
      m_max(INT_MAX, INT_MAX),
      m_foundPoint(target),
      m_foundObject(),
      m_markedOnly(false),
      m_minDistance(0x7FFFFFFF),
      m_config(config)
{
    // ex GLockData::GLockData
}

void
game::map::Locker::setRangeLimit(Point min, Point max)
{
    m_min = min;
    m_max = max;
}

void
game::map::Locker::setMarkedOnly(bool flag)
{
    m_markedOnly = flag;
}

// /** Add a point to the set.
//     \param pt      coordinates
//     \param marked  true iff the object at these coordinates is marked */
void
game::map::Locker::addPoint(Point pt, bool marked, Reference obj)
{
    // ex GLockData::addPoint
    if (m_markedOnly && !marked) {
        return;
    }

    switch (m_config.getMode()) {
     case Configuration::Flat:
     case Configuration::Wrapped:
        // Fast version
        addPointRaw(m_config.getSimpleNearestAlias(pt, m_target), obj);
        break;

     case Configuration::Circular:
        // Generic version (would work for Flat/Wrapped as well but be slower)
        for (int i = 0, n = m_config.getNumPointImages(); i < n; ++i) {
            Point tmp;
            if (m_config.getPointAlias(pt, tmp, i, true)) {
                addPointRaw(tmp, obj);
            }
        }
        break;
    }
}

// /** Add an object to the set. This uses the object's coordinates and selection status.
//     \param obj the object */
void
game::map::Locker::addObject(const MapObject& obj, Reference::Type type)
{
    // ex GLockData::addObject
    Point pt;
    if (obj.getPosition(pt)) {
        addPoint(pt, obj.isMarked(), Reference(type, obj.getId()));
    }
}

// /** Lock onto planets. Processes all planets in /univ/ using the lock
//     data /data/. */
void
game::map::Locker::addPlanets(const Universe& univ)
{
    // ex findPlanet
    AnyPlanetType ty(const_cast<Universe&>(univ));
    for (Id_t pid = ty.findNextIndex(0); pid != 0; pid = ty.findNextIndex(pid)) {
        if (const Planet* pl = ty.getObjectByIndex(pid)) {
            addObject(*pl, Reference::Planet);
        }
    }
}

// /** Lock onto ships. Processes all ships in /univ/ using the lock data
//     /data/. */
void
game::map::Locker::addShips(const Universe& univ)
{
    // ex findShip
    AnyShipType ty(const_cast<Universe&>(univ));
    for (Id_t sid = ty.findNextIndex(0); sid != 0; sid = ty.findNextIndex(sid)) {
        if (const Ship* sh = ty.getObjectByIndex(sid)) {
            addObject(*sh, Reference::Ship);
        }
    }
}

// /** Lock onto Ufo. Processes all ufos in /univ/ using the lock data
//     /data/. */
void
game::map::Locker::addUfos(const Universe& univ)
{
    // ex findUfo
    UfoType& ty(const_cast<UfoType&>(univ.ufos()));
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        if (const Ufo* u = ty.getObjectByIndex(i)) {
            addObject(*u, Reference::Ufo);
        }
    }
}

// /** Lock onto minefields. Processes all minefields in /univ/ using the
//     lock data /data/. */
void
game::map::Locker::addMinefields(const Universe& univ)
{
    // ex findMinefield
    MinefieldType& ty(const_cast<MinefieldType&>(univ.minefields()));
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        if (const Minefield* mf = ty.getObjectByIndex(i)) {
            addObject(*mf, Reference::Minefield);
        }
    }
}

// /** Lock onto marker. Processes all markers (=drawings of type MarkerDrawing)
//     from /univ/ using lock data /data/. */
void
game::map::Locker::addDrawings(const Universe& univ, const Drawing* ignore)
{
    // ex findMarker
    const DrawingContainer& d = univ.drawings();
    for (DrawingContainer::Iterator_t i = d.begin(); i != d.end(); ++i) {
        if (Drawing* pd = *i) {
            if (pd != ignore) {
                if (pd->isVisible() && pd->getType() == Drawing::MarkerDrawing) {
                    addPoint(pd->getPos(), false);
                }
            }
        }
    }

    ExplosionType& e = const_cast<ExplosionType&>(univ.explosions());
    for (Id_t i = e.findNextIndex(0); i != 0; i = e.findNextIndex(i)) {
        if (const Explosion* pe = e.getObjectByIndex(i)) {
            // FIXME: we cannot create references to explosions yet
            addObject(*pe, Reference::Null);
        }
    }
}

// /** Main entry point: do locking according to mode.
//     \param univ   universe to process
//     \param data   GLockData object that does the work
//     \param mode   event, corresponds to the mouse button the user pressed
//     \see setLockMode() */
void
game::map::Locker::addUniverse(const Universe& univ, int32_t items, const Drawing* ignoreDrawing)
{
    if ((items & MatchPlanets) != 0) {
        addPlanets(univ);
    }
    if ((items & MatchShips) != 0) {
        addShips(univ);
    }
    if ((items & MatchUfos) != 0) {
        addUfos(univ);
    }
    if ((items & MatchDrawings) != 0) {
        addDrawings(univ, ignoreDrawing);
    }
    if ((items & MatchMinefields) != 0) {
        addMinefields(univ);
    }
}


// /** Get found point. If the found object is across a map border,
//     this will return the coordinates mapped into the map instance
//     of /clicked/. Do not assume that this is one of the points
//     added with addPoint(); in case none was in range, the clicked
//     point is returned as is. */
game::map::Point
game::map::Locker::getFoundPoint() const
{
    return m_foundPoint;
}

// /** Get found object. May be null if the found point does not
//     correspond to an object. */
game::Reference
game::map::Locker::getFoundObject() const
{
    return m_foundObject;
}

// /** Check point for inclusion in result. Does not mangle the point any
//     further, just checks it. */
void
game::map::Locker::addPointRaw(Point pt, Reference obj)
{
    // ex GLockData::addPointRaw
    if (pt.getX() >= m_min.getX() && pt.getY() >= m_min.getY() && pt.getX() <= m_max.getX() && pt.getY() <= m_max.getY()) {
        int32_t dist = m_config.getSquaredDistance(pt, m_target);
        if (dist < m_minDistance) {
            m_foundPoint  = pt;
            m_foundObject = obj;
            m_minDistance = dist;
        }
    }
}
