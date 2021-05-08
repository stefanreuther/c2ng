/**
  *  \file game/vcr/flak/object.cpp
  *  \brief Class game::vcr::flak::Object
  */

#include "game/vcr/flak/object.hpp"
#include "game/vcr/flak/configuration.hpp"
#include "game/vcr/flak/definitions.hpp"

namespace {
    /*
     *  Formulas
     */

    int32_t computeRating(const game::vcr::flak::Object& ship, const game::vcr::flak::Configuration& config)
    {
        return ship.getMass() * config.RatingMassScale
            + ship.getNumLaunchers() * ship.getTorpedoType() * config.RatingTorpScale
            + ship.getNumBeams() * ship.getBeamType() * config.RatingBeamScale
            + ship.getNumBays() * config.RatingBayScale;
    }

    int computeStrength(const game::vcr::flak::Object& ship, const game::vcr::flak::Configuration& config)
    {
        int32_t strength = config.CompensationShipScale
            + ship.getNumLaunchers() * config.CompensationTorpScale
            + ship.getNumBeams() * config.CompensationBeamScale
            + ship.getNumBays() * config.CompensationFighterScale
            + ship.getMass() * config.CompensationMass100KTScale / 100;
        return (strength > config.CompensationLimit
                ? config.CompensationLimit
                : strength < 0
                ? 0
                : strength);
    }
}

game::vcr::flak::Object::Object()
    : game::vcr::Object(),
      m_maxFightersLaunched(0),
      m_rating(0),
      m_compensation(0),
      m_endingStatus(0)
{ }

game::vcr::flak::Object::Object(const game::vcr::flak::structures::Ship& ship, afl::charset::Charset& charset)
    : game::vcr::Object(),
      m_maxFightersLaunched(ship.maxFightersLaunched),
      m_rating(ship.rating),
      m_compensation(ship.compensation),
      m_endingStatus(ship.endingStatus)
{
    setName(charset.decode(ship.name));
    setDamage(ship.damage);
    setCrew(ship.crew);
    setId(ship.id);
    setOwner(ship.owner);
    setHull(ship.hull);
    setExperienceLevel(ship.experienceLevel);
    setNumBeams(ship.numBeams);
    setBeamType(ship.beamType);
    setNumLaunchers(ship.numLaunchers);
    setNumTorpedoes(ship.numTorpedoes);
    setTorpedoType(ship.torpedoType);
    setNumBays(ship.numBays);
    setNumFighters(ship.numFighters);
    setMass(ship.mass);
    setShield(ship.shield);
    setIsPlanet((ship.flags & structures::flak_IsPlanet) != 0);

    // Missing:
    //   setRace - optional, always taken from pconfig
    //   setPicture - SHOULD not be relevant (but classic combat case uses it; check/FIXME!)
    //   setBeamKillRate, setBeamChargeRate, setTorpMissRate, setTorpChargeRate, setCrewDefenseRate
}

void
game::vcr::flak::Object::pack(game::vcr::flak::structures::Ship& ship, afl::charset::Charset& charset) const
{
    ship.name                  = charset.encode(afl::string::toMemory(getName()));
    ship.damage                = static_cast<int16_t>(getDamage());
    ship.crew                  = static_cast<int16_t>(getCrew());
    ship.id                    = static_cast<int16_t>(getId());
    ship.owner                 = static_cast<int16_t>(getOwner());
    ship.hull                  = static_cast<int16_t>(getHull());
    ship.experienceLevel       = static_cast<int16_t>(getExperienceLevel());
    ship.numBeams              = static_cast<int16_t>(getNumBeams());
    ship.beamType              = static_cast<int16_t>(getBeamType());
    ship.numLaunchers          = static_cast<int16_t>(getNumLaunchers());
    ship.numTorpedoes          = static_cast<int16_t>(getNumTorpedoes());
    ship.torpedoType           = static_cast<int16_t>(getTorpedoType());
    ship.numBays               = static_cast<int16_t>(getNumBays());
    ship.numFighters           = static_cast<int16_t>(getNumFighters());
    ship.mass                  = static_cast<int16_t>(getMass());
    ship.shield                = static_cast<int16_t>(getShield());
    ship.maxFightersLaunched   = static_cast<int16_t>(getMaxFightersLaunched());
    ship.rating                = getRating();
    ship.compensation          = static_cast<int16_t>(getCompensation());
    ship.flags                 = isPlanet() ? structures::flak_IsPlanet : 0;
    ship.endingStatus          = static_cast<int16_t>(getEndingStatus());
}

int
game::vcr::flak::Object::getMaxFightersLaunched() const
{
    return m_maxFightersLaunched;
}

int32_t
game::vcr::flak::Object::getRating() const
{
    return m_rating;
}

int
game::vcr::flak::Object::getCompensation() const
{
    return m_compensation;
}

int
game::vcr::flak::Object::getEndingStatus() const
{
    return m_endingStatus;
}

void
game::vcr::flak::Object::setMaxFightersLaunched(int n)
{
    m_maxFightersLaunched = n;
}

void
game::vcr::flak::Object::setRating(int32_t rating)
{
    m_rating = rating;
}

void
game::vcr::flak::Object::setCompensation(int comp)
{
    m_compensation = comp;
}

void
game::vcr::flak::Object::setEndingStatus(int status)
{
    m_endingStatus = status;
}

void
game::vcr::flak::Object::init(const Configuration& config)
{
    // ex FlakBattle::initShip
    setMaxFightersLaunched(FLAK_MFL_SCALE * getNumBays());
    if (getMaxFightersLaunched() < FLAK_MINIMUM_MFL) {
        setMaxFightersLaunched(FLAK_MINIMUM_MFL);
    }
    if (getMaxFightersLaunched() > FLAK_MAXIMUM_MFL) {
        setMaxFightersLaunched(FLAK_MAXIMUM_MFL);
    }

    setRating(computeRating(*this, config));
    setCompensation(computeStrength(*this, config));
    setEndingStatus(0);
}
