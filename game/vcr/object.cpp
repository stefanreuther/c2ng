/**
  *  \file game/vcr/object.cpp
  *  \brief Class game::vcr::Object
  */

#include "game/vcr/object.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/engine.hpp"

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

int
game::vcr::Object::getGuessedEngine(const game::spec::EngineVector_t& engines,
                                    const game::spec::Hull* pAssumedHull,
                                    bool withESB,
                                    const game::config::HostConfiguration& config) const
{
    // ex client/widgets/vcrinfomain.cc:identifyEngine
    // Don't guess if we don't know the hull
    if (isPlanet() || pAssumedHull == 0) {
        return 0;
    }

    // Compute effective ESB.
    int32_t esb;
    if (withESB) {
        esb = config[config.EngineShieldBonusRate](getOwner());
    } else {
        esb = 0;
    }

    if (config[config.NumExperienceLevels]() > 0 && getExperienceLevel() > 0) {
        esb += config[config.EModEngineShieldBonusRate](getExperienceLevel());
    }

    // Figure out mass that must be accounted for by ESB
    int32_t massDiff = getMass() - pAssumedHull->getMass();
    if (config.getPlayerRaceNumber(getOwner()) == 1) {
        // Scotty bonus
        massDiff -= 50;
    }

    // Is 360 kt bonus applicable?
    bool is360 = (getMass() > 140+360 && getNumBays() > 0);

    int result = 0;
    for (int i = 1, n = engines.size(); i <= n; ++i) {
        if (const game::spec::Engine* p = engines.get(i)) {
            int32_t thisESB = esb * p->cost().get(game::spec::Cost::Money) / 100;
            int32_t remain = massDiff - thisESB;
            if (remain == 0 || (is360 && remain == 360)) {
                if (result != 0) {
                    return 0;
                } else {
                    result = i;
                }
            }
        }
    }
    return result;
}

// Check for freighter.
bool
game::vcr::Object::isFreighter() const
{
    // ex GVcrObject::isFreighter
    return getNumBeams() == 0
        && getNumLaunchers() == 0
        && getNumBays() == 0;
}

// Apply classic shield limits.
void
game::vcr::Object::applyClassicLimits()
{
    // ex GVcrObject::applyClassicLimits
    setShield(std::max(0, std::min(getShield(), 100 - getDamage())));

    if (!isPlanet()) {
        if (isFreighter()) {
            setShield(0);
        }
    } else {
        if (getCrew() <= 0) {
            setShield(0);
        }
    }
}
