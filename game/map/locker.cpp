/**
  *  \file game/map/locker.cpp
  *  \brief Class game::map::Locker
  */

#include <climits>
#include "game/map/locker.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/configuration.hpp"
#include "game/map/object.hpp"
#include "game/map/shippredictor.hpp"
#include "game/map/universe.hpp"
#include "game/spec/engine.hpp"

using game::config::HostConfiguration;

game::map::Locker::Locker(Point target, const Configuration& config)
    : m_target(target),
      m_min(INT_MIN, INT_MIN),
      m_max(INT_MAX, INT_MAX),
      m_tagFilter(),
      m_foundPoint(target),
      m_foundObject(),
      m_markedOnly(false),
      m_minDistance(0x7FFFFFFF),
      m_config(config)
{
    // ex GLockData::GLockData
}

// Set range limit.
void
game::map::Locker::setRangeLimit(Point min, Point max)
{
    m_min = min;
    m_max = max;
}

// Set tag filter.
void
game::map::Locker::setDrawingTagFilter(afl::base::Optional<util::Atom_t> tagFilter)
{
    m_tagFilter = tagFilter;
}

// Set limitation to marked objects.
void
game::map::Locker::setMarkedOnly(bool flag)
{
    m_markedOnly = flag;
}

// Add single point candidate.
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

// Add object candidate.
void
game::map::Locker::addObject(const Object& obj, Reference::Type type)
{
    // ex GLockData::addObject
    Point pt;
    if (obj.getPosition().get(pt)) {
        addPoint(pt, obj.isMarked(), type == Reference::Null ? Reference() : Reference(type, obj.getId()));
    }
}

// Add planets.
void
game::map::Locker::addPlanets(const Universe& univ)
{
    // ex findPlanet
    // ex find.pas:FindPlanet
    const AnyPlanetType& ty(univ.allPlanets());
    for (Id_t pid = ty.findNextIndex(0); pid != 0; pid = ty.findNextIndex(pid)) {
        if (const Planet* pl = univ.planets().get(pid)) {
            addObject(*pl, Reference::Planet);
        }
    }
}

// Add ships.
void
game::map::Locker::addShips(const Universe& univ)
{
    // ex findShip
    // ex find.pas:FindShip, FindShipOrMarker
    const AnyShipType& ty(univ.allShips());
    for (Id_t sid = ty.findNextIndex(0); sid != 0; sid = ty.findNextIndex(sid)) {
        if (const Ship* sh = univ.ships().get(sid)) {
            addObject(*sh, Reference::Ship);
        }
    }
}

// Add Ufos.
void
game::map::Locker::addUfos(const Universe& univ)
{
    // ex findUfo
    // ex find.pas:FindPlanetOrUfo
    // FIXME(?): handle outside points for circular objects?
    UfoType& ty(const_cast<UfoType&>(univ.ufos()));
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        if (const Ufo* u = ty.getObjectByIndex(i)) {
            addObject(*u, Reference::Ufo);
        }
    }
}

// Add minefields.
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

// Add drawings.
void
game::map::Locker::addDrawings(const Universe& univ, const Drawing* ignore)
{
    // ex findMarker
    const DrawingContainer& d = univ.drawings();
    const util::Atom_t* pTag = m_tagFilter.get();
    for (DrawingContainer::Iterator_t i = d.begin(); i != d.end(); ++i) {
        if (Drawing* pd = *i) {
            if (pd != ignore) {
                if (pd->isVisible() && pd->getType() == Drawing::MarkerDrawing && (pTag == 0 || *pTag == pd->getTag())) {
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

// Add universe (main entry point).
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

// Find warp-well edge.
game::map::Point
game::map::Locker::findWarpWellEdge(Point origin,
                                    bool isHyperdriving,
                                    const Universe& univ,
                                    Id_t shipId,
                                    const UnitScoreDefinitionList& scoreDefinitions,
                                    const game::spec::ShipList& shipList,
                                    const game::config::HostConfiguration& config,
                                    const HostVersion& host,
                                    const RegistrationKey& key) const
{
    // ex WScannerChartWidget::doItemLock (part)
    // Query current position
    Id_t originPlanetId = univ.findGravityPlanetAt(origin, m_config, config, host);

    // Can we optimize warp wells?
    Id_t foundPlanetId = univ.findPlanetAt(m_config.getCanonicalLocation(m_foundPoint));
    if (foundPlanetId != 0
        && config[HostConfiguration::AllowGravityWells]()
        && (!isHyperdriving || !host.isPHost() || config[HostConfiguration::AllowHyperjumpGravWells]())
        && foundPlanetId != originPlanetId)
    {
        /* We try to find the edge of a gravity well unless
           - we're heading for deep space, i.e. no planet found
           - gravity wells are disabled
           - we're starting inside the same gravity well we clicked in,
             in this case we assume we want to move to the planet */
        // FIXME: check whether this still matches actual rules!
        const int wwrange = (host.isPHost() ? config[HostConfiguration::GravityWellRange]() :
                             isHyperdriving ? 2 : 3);

        // Start with the assumption that moving directly is the best choice.
        // Then try all points in warp well range.
        int32_t bestDistance = getWarpWellDistanceMetric(origin, m_foundPoint, isHyperdriving, univ, shipId, scoreDefinitions, shipList, config, host, key);
        Point bestPoint  = m_foundPoint;
        for (int dx = -wwrange; dx <= wwrange; ++dx) {
            for (int dy = -wwrange; dy <= wwrange; ++dy) {
                Point newPoint(m_foundPoint.getX() + dx, m_foundPoint.getY() + dy);
                int32_t newDistance = getWarpWellDistanceMetric(origin, newPoint, isHyperdriving, univ, shipId, scoreDefinitions, shipList, config, host, key);
                if (newDistance >= 0
                    && (bestDistance < 0 || newDistance < bestDistance)
                    && univ.findGravityPlanetAt(newPoint, m_config, config, host) == foundPlanetId)
                {
                    // Accept new point if it is valid, has a better metric than
                    // the previous one, and it is in the same warp well.
                    bestDistance = newDistance;
                    bestPoint = newPoint;
                }
            }
        }

        // Move to found point, if any
        if (bestDistance >= 0) {
            return bestPoint;
        } else {
            return m_foundPoint;
        }
    } else {
        // No warp wells, so just return found point
        return m_foundPoint;
    }
}

// Get found point.
game::map::Point
game::map::Locker::getFoundPoint() const
{
    return m_foundPoint;
}

// Get found object.
game::Reference
game::map::Locker::getFoundObject() const
{
    return m_foundObject;
}

/* Check point for inclusion in result. Does not mangle the point any further, just checks it. */
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

/* Get warp well distance metric: refuse non-exact hyperjump targets */
int32_t
game::map::Locker::getWarpWellDistanceMetric(Point origin, Point pt,
                                             bool isHyperdriving,
                                             const Universe& univ,
                                             Id_t shipId,
                                             const UnitScoreDefinitionList& scoreDefinitions,
                                             const game::spec::ShipList& shipList,
                                             const game::config::HostConfiguration& config,
                                             const HostVersion& host,
                                             const RegistrationKey& key) const
{
    // ex WShipScannerChartWidget::lockQueryDistance
    int32_t dist2 = m_config.getSquaredDistance(origin, pt);
    if (isHyperdriving && !host.isExactHyperjumpDistance2(dist2)) {
        return -1;
    } else {
        if (game::map::Ship* sh = univ.ships().get(shipId)) {
            ShipPredictor pred(univ, shipId, scoreDefinitions, shipList, m_config, config, host, key);
            if (game::spec::Engine* e = shipList.engines().get(sh->getEngineType().orElse(0))) {
                pred.setWarpFactor(e->getMaxEfficientWarp());
            }
            pred.setPosition(origin);
            pred.setWaypoint(pt);
            pred.computeMovement();
            const int t = pred.getNumTurns();

            // Combining distance and time metric into one value.
            // Better time should trump better distance.
            // - assume a maximum sensible distance of 5000, maximum sensible dist2 is 25M
            // - assume a maximum time of 32 (MOVEMENT_TIME_LIMIT), and a maximum metric of 2G, maximum slice is 62M
            const int MAX_TIME = 32;  // give some headroom to MOVEMENT_TIME_LIMIT
            const int32_t SCALE = 0x7FFFFFFF / MAX_TIME;
            return SCALE * std::min(t, MAX_TIME)
                + std::min(dist2, SCALE-1);
        } else {
            return dist2;
        }
    }
}
