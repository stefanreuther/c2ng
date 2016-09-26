/**
  *  \file game/vcr/object.hpp
  */
#ifndef C2NG_GAME_VCR_OBJECT_HPP
#define C2NG_GAME_VCR_OBJECT_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "game/spec/componentvector.hpp"

namespace game { namespace vcr {

    /** Object taking part in a VCR.
        This describes an object taking part in a fight, containing the basic object specs.
        It has a virtual destructor so that combat algorithms using more than these specs can derive from this class,
        add their own properties, and be still recognizable using RTTI.
        Methods are inline, not virtual, because this is also used at the core of VCR algorithms which are intended to be fast. */
    class Object : public afl::base::Deletable {
     public:
        Object();

        // FIXME: belongs elsewhere
        // GVcrObject(const TRawVcr& raw, int side);

        virtual ~Object();

        //     void unpack(const TRawVcr& raw, int side);
        //     bool pack(TRawVcr& raw, int side);

        int getMass() const;
        int getShield() const;
        int getDamage() const;
        int getCrew() const;
        int getId() const;
        int getOwner() const;
        int getRace() const;
        int getPicture() const;
        int getHull() const;
        int getBeamType() const;
        int getNumBeams() const;
        int getTorpedoType() const;
        int getNumTorpedoes() const;
        int getNumLaunchers() const;
        int getNumBays() const;
        int getNumFighters() const;
        int getExperienceLevel() const;
        int getBeamKillRate() const;
        int getBeamChargeRate() const;
        int getTorpMissRate() const;
        int getTorpChargeRate() const;
        int getCrewDefenseRate() const;

        bool isPlanet() const;
        const String_t& getName() const;

        void setMass(int mass);
        void setShield(int shield);
        void setDamage(int damage);
        void setCrew(int crew);
        void setId(int id);
        void setOwner(int owner);
        void setRace(int raceOrZero);
        void setPicture(int picture);
        void setHull(int hullOrZero);
        void setBeamType(int beamType);
        void setNumBeams(int numBeams);
        void setTorpedoType(int torpedoType);
        void setNumTorpedoes(int numTorpedoes);
        void setNumLaunchers(int numLaunchers);
        void setNumBays(int numBays);
        void setNumFighters(int numFighters);
        void setExperienceLevel(int level);
        void setBeamKillRate(int beamKillRate);
        void setBeamChargeRate(int beamChargeRate);
        void setTorpMissRate(int torpMissRate);
        void setTorpChargeRate(int torpChargeRate);
        void setCrewDefenseRate(int crewDefenseRate);
        void setIsPlanet(bool isPlanet);
        void setName(const String_t& name);

        void addFighters(int n);
        void addTorpedoes(int n);
        void addBays(int n);
        void addMass(int n);

        void setGuessedHull(const game::spec::HullVector_t& hulls);
        bool canBeHull(const game::spec::HullVector_t& hulls, int hullId) const;
        int  getGuessedHull(const game::spec::HullVector_t& hulls) const;
        int  getGuessedShipPicture(const game::spec::HullVector_t& hulls) const;

     private:
        typedef int Value_t;
        // Placing all POD members in a structure causes gcc-3.3 to generate better code for construction/assignment.
        struct {
            Value_t  mass;
            Value_t  shield;
            Value_t  damage;
            Value_t  crew;
            Value_t  id;
            Value_t  owner;
            Value_t  raceOrZero;
            Value_t  picture;
            Value_t  hullOrZero;
            Value_t  beamType;
            Value_t  numBeams;
            Value_t  torpedoType;
            Value_t  numTorpedoes;
            Value_t  numLaunchers;
            Value_t  numBays;
            Value_t  numFighters;
            Value_t  experienceLevel;
            bool     isPlanet;

            // Nu extensions:
            Value_t  beamKillRate;
            Value_t  beamChargeRate;
            Value_t  torpMissRate;
            Value_t  torpChargeRate;
            Value_t  crewDefenseRate;
        } m_data;
        String_t m_name;
    };

} }


/******************************** Inlines ********************************/

inline int
game::vcr::Object::getMass() const
{
    return m_data.mass;
}

inline int
game::vcr::Object::getShield() const
{
    return m_data.shield;
}

inline int
game::vcr::Object::getDamage() const
{
    return m_data.damage;
}

inline int
game::vcr::Object::getCrew() const
{
    return m_data.crew;
}

inline int
game::vcr::Object::getId() const
{
    return m_data.id;
}

inline int
game::vcr::Object::getOwner() const
{
    return m_data.owner;
}

inline int
game::vcr::Object::getRace() const
{
    return m_data.raceOrZero;
}

inline int
game::vcr::Object::getPicture() const
{
    return m_data.picture;
}

inline int
game::vcr::Object::getHull() const
{
    return m_data.hullOrZero;
}

inline int
game::vcr::Object::getBeamType() const
{
    return m_data.beamType;
}

inline int
game::vcr::Object::getNumBeams() const
{
    return m_data.numBeams;
}

inline int
game::vcr::Object::getTorpedoType() const
{
    return m_data.torpedoType;
}

inline int
game::vcr::Object::getNumTorpedoes() const
{
    return m_data.numTorpedoes;
}

inline int
game::vcr::Object::getNumLaunchers() const
{
    return m_data.numLaunchers;
}

inline int
game::vcr::Object::getNumBays() const
{
    return m_data.numBays;
}

inline int
game::vcr::Object::getNumFighters() const
{
    return m_data.numFighters;
}

inline int
game::vcr::Object::getExperienceLevel() const
{
    return m_data.experienceLevel;
}

inline int
game::vcr::Object::getBeamKillRate() const
{
    return m_data.beamKillRate;
}

inline int
game::vcr::Object::getBeamChargeRate() const
{
    return m_data.beamChargeRate;
}

inline int
game::vcr::Object::getTorpMissRate() const
{
    return m_data.torpMissRate;
}

inline int
game::vcr::Object::getTorpChargeRate() const
{
    return m_data.torpChargeRate;
}

inline int
game::vcr::Object::getCrewDefenseRate() const
{
    return m_data.crewDefenseRate;
}

inline bool
game::vcr::Object::isPlanet() const
{
    return m_data.isPlanet;
}

inline const String_t&
game::vcr::Object::getName() const
{
    return m_name;
}

inline void
game::vcr::Object::setMass(int mass)
{
    m_data.mass = mass;
}

inline void
game::vcr::Object::setShield(int shield)
{
    m_data.shield = shield;
}

inline void
game::vcr::Object::setDamage(int damage)
{
    m_data.damage = damage;
}

inline void
game::vcr::Object::setCrew(int crew)
{
    m_data.crew = crew;
}

inline void
game::vcr::Object::setId(int id)
{
    m_data.id = id;
}

inline void
game::vcr::Object::setOwner(int owner)
{
    m_data.owner = owner;
}

inline void
game::vcr::Object::setRace(int raceOrZero)
{
    m_data.raceOrZero = raceOrZero;
}

inline void
game::vcr::Object::setPicture(int picture)
{
    m_data.picture = picture;
}

inline void
game::vcr::Object::setHull(int hullOrZero)
{
    m_data.hullOrZero = hullOrZero;
}

inline void
game::vcr::Object::setBeamType(int beamType)
{
    m_data.beamType = beamType;
}

inline void
game::vcr::Object::setNumBeams(int numBeams)
{
    m_data.numBeams = numBeams;
}

inline void
game::vcr::Object::setTorpedoType(int torpedoType)
{
    m_data.torpedoType = torpedoType;
}

inline void
game::vcr::Object::setNumTorpedoes(int numTorpedoes)
{
    m_data.numTorpedoes = numTorpedoes;
}

inline void
game::vcr::Object::setNumLaunchers(int numLaunchers)
{
    m_data.numLaunchers = numLaunchers;
}

inline void
game::vcr::Object::setNumBays(int numBays)
{
    m_data.numBays = numBays;
}

inline void
game::vcr::Object::setNumFighters(int numFighters)
{
    m_data.numFighters = numFighters;
}

inline void
game::vcr::Object::setExperienceLevel(int level)
{
    m_data.experienceLevel = level;
}

inline void
game::vcr::Object::setBeamKillRate(int beamKillRate)
{
    m_data.beamKillRate = beamKillRate;
}

inline void
game::vcr::Object::setBeamChargeRate(int beamChargeRate)
{
    m_data.beamChargeRate = beamChargeRate;
}

inline void
game::vcr::Object::setTorpMissRate(int torpChargeRate)
{
    m_data.torpMissRate = torpChargeRate;
}

inline void
game::vcr::Object::setTorpChargeRate(int torpChargeRate)
{
    m_data.torpChargeRate = torpChargeRate;
}

inline void
game::vcr::Object::setCrewDefenseRate(int crewDefenseRate)
{
    m_data.crewDefenseRate = crewDefenseRate;
}

inline void
game::vcr::Object::setIsPlanet(bool isPlanet)
{
    m_data.isPlanet = isPlanet;
}

inline void
game::vcr::Object::setName(const String_t& name)
{
    m_name = name;
}

inline void
game::vcr::Object::addFighters(int n)
{
    m_data.numFighters += n;
}

inline void
game::vcr::Object::addTorpedoes(int n)
{
    m_data.numTorpedoes += n;
}

inline void
game::vcr::Object::addBays(int n)
{
    m_data.numBays += n;
}

inline void
game::vcr::Object::addMass(int n)
{
    m_data.mass += n;
}

#endif
