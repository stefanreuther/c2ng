/**
  *  \file game/vcr/object.cpp
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

// /** Remember guessed hull. This will modify the object in place, so it's a bad
//     idea to call this on VCRs you intend to export, but it'll save a few cycles
//     in guessHull() later on. */
void
game::vcr::Object::setGuessedHull(const game::spec::HullVector_t& hulls)
{
    // ex GVcrObject::setGuessedHull()
    setHull(getGuessedHull(hulls));
}

// /** Check if this could be the specified hull.
//     \param hull_id Hull to check for
//     \return true iff this ship could be of the specified hull
//     \pre isPlanet() */
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
// FIXME: incomplete port
//         /* This checks the same properties as PCC 1.x does. It does not check:
//            - Mass. Normally, the ship shouldn't be lighter than its hull's mass,
//              but since balancing approaches toy around with the mass, we don't
//              trust it too much.
//            - Crew. The crew can be larger (tow-capture bug) or smaller than
//              the hull's standard crew. */

//         /* picture must match.
//            THost has an easter egg where it reports Nebulas (picture 16) with picture 30
//            instead when they have Transwarp Drives. */
//         if (getPicture() != hull.getExternalPictureNumber()
//             && (getPicture() != 30
//                 || hull.getExternalPictureNumber() != 16))
//             return false;
//         /* must not have more beams/torps than hull allows */
//         if (getBeamCount() > hull.getMaxBeams() || getTorpLauncherCount() > hull.getMaxLaunchers())
//             return false;
//         /* for fighter bays, the only criterion is that ship has
//            fighters but hull has not. The number of bays can be
//            smaller (NTP, damage) or larger (scotty bonus). */
//         if (getBayCount() && !hull.getNumBays())
//             return false;
        return true;
    }
}

// /** Guess this ship's hull. Whereas getHull() returns the value from the combat
//     record, which can be missing, this will attempt to guess the correct value.
//     \return hull number, zero if ambiguous, impossible, or this is actually a planet */
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
    for (int i = 1, n = hulls.size(); i <= n; ++i) {
        if (canBeHull(hulls, i)) {
            if (type == 0) {
                type = i;
            } else {
                return 0;       // ambiguous, can't guess
            }
        }
    }
    return type;
}

// /** Get ship picture. Whereas getPicture() just returns the value from the combat record,
//     this will attempt to resolve the record back into a ship type, and give that ship's
//     picture. Thus, it will reflect users' changes. */
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
