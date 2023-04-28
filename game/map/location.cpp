/**
  *  \file game/map/location.cpp
  *  \brief Class game::map::Location
  */

#include "game/map/location.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/configuration.hpp"
#include "game/map/playedplanettype.hpp"
#include "game/map/playedshiptype.hpp"
#include "game/map/universe.hpp"

namespace {
    afl::base::Optional<game::map::Point> getPositionFromReference(const game::map::Universe* pUniv, const game::Reference ref)
    {
        // Try to resolve as object
        if (pUniv != 0) {
            if (const game::map::Object* p = pUniv->getObject(ref)) {
                return p->getPosition();
            }
        }

        // Could still be a reference to a map location
        return ref.getPosition();
    }
}


game::map::Location::Location()
    : sig_positionChange(),
      m_pUniverse(0),
      m_pConfig(0),
      m_point(),
      m_reference(),
      m_pointValid(false)
{ }

void
game::map::Location::setUniverse(Universe* univ, const Configuration* mapConfig)
{
    // Save old position in case the object does not exist in the new universe
    if (getPositionFromReference(m_pUniverse, m_reference).get(m_point)) {
        m_pointValid = true;
    }

    // Update
    m_pUniverse = univ;
    m_pConfig = mapConfig;
}

void
game::map::Location::set(Reference ref)
{
    // ex GChartLocation::setCurrentObject
    afl::base::Optional<Point> lastPos = getPosition();

    m_reference = ref;

    // Set point to position from reference, unless it already is an alias of the current position
    Point pt;
    if (getPositionFromReference(m_pUniverse, m_reference).get(pt)) {
        if (!(m_pConfig != 0 && m_pointValid && m_pConfig->getCanonicalLocation(m_point) == pt)) {
            m_point = pt;
            m_pointValid = true;
        }
    }

    notifyListeners(lastPos);
}

void
game::map::Location::set(Point pt)
{
    afl::base::Optional<Point> lastPos = getPosition();

    m_point = pt;
    m_pointValid = true;
    m_reference = Reference();

    notifyListeners(lastPos);
}

afl::base::Optional<game::map::Point>
game::map::Location::getPosition() const
{
    Point pt;
    if (getPositionFromReference(m_pUniverse, m_reference).get(pt)) {
        // If point represents an alias of the current position, report that instead
        if (m_pConfig != 0 && m_pointValid && m_pConfig->getCanonicalLocation(m_point) == pt) {
            pt = m_point;
        }
        return pt;
    } else if (m_pointValid) {
        return m_point;
    } else {
        return afl::base::Nothing;
    }
}

afl::base::Optional<game::map::Point>
game::map::Location::getOtherPosition(Id_t shipId) const
{
    // ex GChartLocation::getOtherPosition
    Point currentPosition;
    if (getPosition().get(currentPosition)) {
        if (m_pUniverse != 0 && m_pConfig != 0) {
            if (const Ship* sh = m_pUniverse->ships().get(shipId)) {
                // - if position is at ship, return its waypoint
                Point pt, result;
                if (sh->getPosition().get(pt) && pt == currentPosition && sh->getWaypoint().get(result) && result != currentPosition) {
                    return result;
                }
                // - if position is at ship waypoint, return its position
                if (sh->getWaypoint().get(pt) && pt == currentPosition && sh->getPosition().get(result) && result != currentPosition) {
                    return result;
                }
            }

            // - if position is in a wormhole, return exit position
            // ex chartusr.pas:ChartUfoX
            for (Id_t i = m_pUniverse->ufos().findNextIndex(0); i != 0; i = m_pUniverse->ufos().findNextIndex(i)) {
                if (const Ufo* u = m_pUniverse->ufos().getObjectByIndex(i)) {
                    Point ufoCenter;
                    int32_t ufoRadius;
                    if (u->getPosition().get(ufoCenter) && u->getRadiusSquared().get(ufoRadius) && m_pConfig->getSquaredDistance(currentPosition, ufoCenter) <= ufoRadius) {
                        if (const Ufo* other = u->getOtherEnd()) {
                            Point result;
                            if (other->getPosition().get(result) && result != currentPosition) {
                                return result;
                            }
                        }
                    }
                }
            }

            // - if circular map is active, switch between map images
            if (m_pConfig->getMode() == Configuration::Circular) {
                Point result;

                // inside > out
                if (m_pConfig->getPointAlias(currentPosition, result, 1, true)) {
                    return result;
                }

                // outside > in
                result = m_pConfig->getCanonicalLocation(currentPosition);
                if (result != currentPosition) {
                    return result;
                }
            }
        }
    }

    // No match
    return afl::base::Nothing;
}

game::Reference
game::map::Location::getReference() const
{
    return m_reference;
}

game::Reference
game::map::Location::getEffectiveReference() const
{
    return getPositionFromReference(m_pUniverse, m_reference).isValid()
        ? m_reference
        : Reference();
}

void
game::map::Location::browse(BrowseFlags_t flags)
{
    // ex client/chart/standardmode.cc:doPage(GChartLocation& loc, int delta, bool marked_only, bool any)
    // Fetch object
    if (m_pUniverse == 0) {
        return;
    }
    const Object* obj = m_pUniverse->getObject(m_reference);
    if (obj == 0) {
        return;
    }

    // Get associated object type
    ObjectType* ty = 0;
    if (const Ship* sh = dynamic_cast<const Ship*>(obj)) {
        // Iterate through all ships or player ships
        if (sh->isPlayable(Object::ReadOnly) && flags.contains(PlayedOnly)) {
            ty = &m_pUniverse->playedShips();
        } else {
            ty = &m_pUniverse->allShips();
        }
    } else if (const Planet* pl = dynamic_cast<const Planet*>(obj)) {
        // Iterate through all planets or player planets
        if (pl->isPlayable(Object::ReadOnly) && flags.contains(PlayedOnly)) {
            ty = &m_pUniverse->playedPlanets();
        } else {
            ty = &m_pUniverse->allPlanets();
        }
    } else {
        // Not known
        // @change This cannot currently happen; PCC2 would iterate through whatever-it-is
    }

    // Get next object
    if (ty != 0) {
        Id_t next = (flags.contains(Backwards)
                     ? ty->findPreviousIndexWrap(obj->getId(), flags.contains(MarkedOnly))
                     : ty->findNextIndexWrap    (obj->getId(), flags.contains(MarkedOnly)));
        if (next != 0) {
            set(Reference(m_reference.getType(), next));
        }
    }
}

void
game::map::Location::notifyListeners(const afl::base::Optional<Point>& lastPos)
{
    afl::base::Optional<Point> thisPos = getPosition();
    if (!thisPos.isSame(lastPos)) {
        sig_positionChange.raise(thisPos.orElse(Point()));
    }
}
