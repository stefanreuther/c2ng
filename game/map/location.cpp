/**
  *  \file game/map/location.cpp
  *  \brief Class game::map::Location
  */

#include "game/map/location.hpp"
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
      m_point(),
      m_reference(),
      m_pointValid(false)
{ }

void
game::map::Location::setUniverse(Universe* univ)
{
    // Save old position in case the object does not exist in the new universe
    if (getPositionFromReference(m_pUniverse, m_reference, m_point)) {
        m_pointValid = true;
    }

    // Update
    m_pUniverse = univ;
}

void
game::map::Location::set(Reference ref)
{
    // ex GChartLocation::setCurrentObject
    Point lastPos;
    bool  lastOK = getPosition(lastPos);

    m_reference = ref;

    notifyObservers(lastOK, lastPos);
}

void
game::map::Location::set(Point pt)
{
    Point lastPos;
    bool  lastOK = getPosition(lastPos);

    m_point = pt;
    m_pointValid = true;
    m_reference = Reference();

    notifyObservers(lastOK, lastPos);
}

bool
game::map::Location::getPosition(Point& pt) const
{
    if (getPositionFromReference(m_pUniverse, m_reference, pt)) {
        // Note that this means if an object becomes invisible within its universe,
        // we fall back to the last point, not to the last known position of this object.
        // I don't expect this to happen normally.
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

void
game::map::Location::notifyObservers(bool lastOK, Point lastPos)
{
    Point thisPos;
    bool  thisOK = getPosition(thisPos);

    if (lastOK != thisOK || lastPos != thisPos) {
        sig_positionChange.raise(thisPos);
    }
}
