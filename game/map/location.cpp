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
    bool getPositionFromReference(const game::map::Universe* pUniv, const game::Reference ref, game::map::Point& out)
    {
        // Try to resolve as object
        if (pUniv != 0) {
            if (const game::map::Object* p = pUniv->getObject(ref)) {
                return p->getPosition(out);
            }
        }

        // Could still be a reference to a map location
        return ref.getPos(out);
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
    if (getPositionFromReference(m_pUniverse, m_reference, m_point)) {
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
    Point lastPos;
    bool  lastOK = getPosition(lastPos);

    m_reference = ref;

    // Set point to position from reference, unless it already is an alias of the current position
    Point pt;
    if (getPositionFromReference(m_pUniverse, m_reference, pt)) {
        if (!(m_pConfig != 0 && m_pointValid && m_pConfig->getCanonicalLocation(m_point) == pt)) {
            m_point = pt;
            m_pointValid = true;
        }
    }

    notifyListeners(lastOK, lastPos);
}

void
game::map::Location::set(Point pt)
{
    Point lastPos;
    bool  lastOK = getPosition(lastPos);

    m_point = pt;
    m_pointValid = true;
    m_reference = Reference();

    notifyListeners(lastOK, lastPos);
}

bool
game::map::Location::getPosition(Point& pt) const
{
    if (getPositionFromReference(m_pUniverse, m_reference, pt)) {
        // If point represents an alias of the current position, report that instead
        if (m_pConfig != 0 && m_pointValid && m_pConfig->getCanonicalLocation(m_point) == pt) {
            pt = m_point;
        }
        return true;
    } else if (m_pointValid) {
        pt = m_point;
        return true;
    } else {
        return false;
    }
}

game::Reference
game::map::Location::getReference() const
{
    return m_reference;
}

game::Reference
game::map::Location::getEffectiveReference() const
{
    Point pt;
    return getPositionFromReference(m_pUniverse, m_reference, pt)
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
    std::auto_ptr<ObjectType> ty;
    if (const Ship* sh = dynamic_cast<const Ship*>(obj)) {
        // Iterate through all ships or player ships
        if (sh->isPlayable(Object::ReadOnly) && flags.contains(PlayedOnly)) {
            ty.reset(new PlayedShipType(m_pUniverse->ships()));
        } else {
            ty.reset(new AnyShipType(m_pUniverse->ships()));
        }
    } else if (const Planet* pl = dynamic_cast<const Planet*>(obj)) {
        // Iterate through all planets or player planets
        if (pl->isPlayable(Object::ReadOnly) && flags.contains(PlayedOnly)) {
            ty.reset(new PlayedPlanetType(m_pUniverse->planets()));
        } else {
            ty.reset(new AnyPlanetType(m_pUniverse->planets()));
        }
    } else {
        // Not known
        // @change This cannot currently happen; PCC2 would iterate through whatever-it-is
    }

    // Get next object
    if (ty.get() != 0) {
        Id_t next = (flags.contains(Backwards)
                     ? ty->findPreviousIndexWrap(obj->getId(), flags.contains(MarkedOnly))
                     : ty->findNextIndexWrap    (obj->getId(), flags.contains(MarkedOnly)));
        if (next != 0) {
            set(Reference(m_reference.getType(), next));
        }
    }
}

void
game::map::Location::notifyListeners(bool lastOK, Point lastPos)
{
    Point thisPos;
    bool  thisOK = getPosition(thisPos);

    if (lastOK != thisOK || lastPos != thisPos) {
        sig_positionChange.raise(thisPos);
    }
}
