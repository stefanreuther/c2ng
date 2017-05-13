/**
  *  \file game/vcr/object.cpp
  *  \brief Class game::vcr::Object
  */

#include "game/vcr/object.hpp"
#include "game/spec/hull.hpp"

// /** Default constructor. */
game::vcr::Object::Object()
    : m_data(),
      m_name()
{
    // ex GVcrObject::GVcrObject
    m_data.beamKillRate    = 1;
    m_data.beamChargeRate  = 1;
    m_data.torpMissRate    = 35;
    m_data.torpChargeRate  = 1;
    m_data.crewDefenseRate = 0;
}

/** Destructor. */
game::vcr::Object::~Object()
{
    // ex GVcrObject::~GVcrObject
}

// Remember guessed hull.
void
game::vcr::Object::setGuessedHull(const game::spec::HullVector_t& hulls)
{
    // ex GVcrObject::setGuessedHull()
    setHull(getGuessedHull(hulls));
}

// Check if this could be the specified hull.
bool
game::vcr::Object::canBeHull(const game::spec::HullVector_t& hulls, int hullId) const
{
    // ex GVcrObject::canBeHull
    const game::spec::Hull* theHull = hulls.get(hullId);
    if (theHull == 0) {
        // Hull does not exist
        return false;
    } else if (isPlanet()) {
        // I'm a planet
        return false;
    } else if (getHull() != 0) {
        // Hull is known
        return getHull() == hullId;
    } else {
        /* This checks the same properties as PCC 1.x does. It does not check:
           - Mass. Normally, the ship shouldn't be lighter than its hull's mass,
             but since balancing approaches toy around with the mass, we don't
             trust it too much.
           - Crew. The crew can be larger (tow-capture bug) or smaller than
             the hull's standard crew. */

        /* Picture must match.
           THost has an easter egg where it reports Nebulas (picture 16) with picture 30
           instead when they have Transwarp Drives. */
        if (getPicture() != theHull->getExternalPictureNumber()
            && (getPicture() != 30
                || theHull->getExternalPictureNumber() != 16))
        {
            return false;
        }

        /* Must not have more beams/torps than hull allows */
        if (getNumBeams() > theHull->getMaxBeams() || getNumLaunchers() > theHull->getMaxLaunchers()) {
            return false;
        }

        /* For fighter bays, the only criterion is that ship has fighters but hull has not.
           The number of bays can be smaller (damage), zero (NTP) or larger (scotty bonus). */
        if (getNumBays() != 0 && theHull->getNumBays() == 0) {
            return false;
        }
        return true;
    }
}

// Guess this ship's hull.
int
game::vcr::Object::getGuessedHull(const game::spec::HullVector_t& hulls) const
{
    // ex GVcrObject::getGuessedHull
    // planets don't have hulls
    if (isPlanet()) {
        return 0;
    }

    // see if PHost sent us the hull type
    if (int h = getHull()) {
        return h;
    }

    // otherwise, try all hulls.
    int type = 0;
    for (game::spec::Hull* p = hulls.findNext(0); p != 0; p = hulls.findNext(p->getId())) {
        int id = p->getId();
        if (canBeHull(hulls, id)) {
            if (type == 0) {
                type = id;
            } else {
                return 0;       // ambiguous, can't guess
            }
        }
    }
    return type;
}

int
game::vcr::Object::getGuessedShipPicture(const game::spec::HullVector_t& hulls) const
{
    // ex GVcrObject::getGuessedShipPicture
    if (isPlanet()) {
        return 0;
    } else if (const game::spec::Hull* hull = hulls.get(getGuessedHull(hulls))) {
        return hull->getInternalPictureNumber();
    } else {
        return getPicture();
    }
}
