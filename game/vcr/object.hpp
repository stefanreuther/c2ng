/**
  *  \file game/vcr/object.hpp
  *  \brief Class game::vcr::Object
  */
#ifndef C2NG_GAME_VCR_OBJECT_HPP
#define C2NG_GAME_VCR_OBJECT_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/root.hpp"
#include "game/spec/componentvector.hpp"
#include "game/spec/shiplist.hpp"
#include "game/teamsettings.hpp"
#include "game/vcr/info.hpp"

namespace game { namespace vcr {

    /** Object taking part in a VCR.
        This describes an object taking part in a fight, containing the basic object specs.
        It has a virtual destructor so that combat algorithms using more than these specs can derive from this class,
        add their own properties, and be still recognizable using RTTI.
        Methods are inline, not virtual, because this is also used at the core of VCR algorithms which are intended to be fast.

        Otherwise this is a data container that does not restrict or interpret the data it contains. */
    class Object : public afl::base::Deletable {
     public:
        /** Role of a unit.
            The role has no effect on the battle outcome, but can affect scoring.
            It is normally not known for fights from the game, but interesting to know for simulation. */
        enum Role {
            NoRole,             ///< No particular role/not known.
            AggressorRole,      ///< This unit is the aggressor.
            OpponentRole        ///< This unit is the opponent.
        };


        /** Constructor.
            Makes default-initialized object. */
        Object();

        /** Destructor. */
        virtual ~Object();

        /** Get mass.
            \return mass */
        int getMass() const;

        /** Get shield level.
            \return shield level */
        int getShield() const;

        /** Get damage level.
            \return damage */
        int getDamage() const;

        /** Get crew size.
            \return crew size */
        int getCrew() const;

        /** Get Id.
            \return Id */
        int getId() const;

        /** Get owner.
            \return owner */
        int getOwner() const;

        /** Get race (optional).
            \return race, or zero */
        int getRace() const;

        /** Get picture number.
            \return picture number */
        int getPicture() const;

        /** Get hull Id (optional).
            \return hull Id, or zero */
        int getHull() const;

        /** Get beam type.
            \return beam type (0=none) */
        int getBeamType() const;

        /** Get number of beams.
            \return number of beams */
        int getNumBeams() const;

        /** Get torpedo type.
            \return torpedo type (0=none) */
        int getTorpedoType() const;

        /** Get number of torpedoes.
            \return number of torpedoes */
        int getNumTorpedoes() const;

        /** Get number of torpedo launchers.
            \return number of launchers */
        int getNumLaunchers() const;

        /** Get number of fighter bays.
            \return number of fighter bays */
        int getNumBays() const;

        /** Get number of fighters.
            \return number of fighters */
        int getNumFighters() const;

        /** Get experience level.
            \return experience level */
        int getExperienceLevel() const;

        /** Get beam kill rate.
            \return beam kill rate (default: 1) */
        int getBeamKillRate() const;

        /** Get beam recharge rate.
            \return beam recharge rate (default: 1) */
        int getBeamChargeRate() const;

        /** Get torpedo miss rate.
            \return torpedo miss rate (default: 35) */
        int getTorpMissRate() const;

        /** Get torpedo recharge rate.
            \return torpedo recharge rate (default: 1) */
        int getTorpChargeRate() const;

        /** Get crew defense rate.
            \return crew defense rate (default: 0) */
        int getCrewDefenseRate() const;

        /** Check for planet.
            \retval true this is a planet
            \retval false this is a ship */
        bool isPlanet() const;

        /** Get name.
            \return name */
        const String_t& getName() const;

        /** Get role.
            \return role */
        Role getRole() const;

        /** Set mass.
            \param mass mass */
        void setMass(int mass);

        /** Set shield level.
            \param shield shield level */
        void setShield(int shield);

        /** Set damage level.
            \param damage damage */
        void setDamage(int damage);

        /** Set crew size.
            \param crew crew size */
        void setCrew(int crew);

        /** Set Id.
            \param id Id */
        void setId(int id);

        /** Set owner.
            \param owner owner */
        void setOwner(int owner);

        /** Set race.
            \param raceOrZero race (zero: default) */
        void setRace(int raceOrZero);

        /** Set picture number.
            \param picture picture number */
        void setPicture(int picture);

        /** Set hull Id (optional).
            \param hullOrZero hull Id (zero: unknown) */
        void setHull(int hullOrZero);

        /** Set beam type.
            \param beamType beam type (0=none) */
        void setBeamType(int beamType);

        /** Set number of beams.
            \param numBeams number of beams */
        void setNumBeams(int numBeams);

        /** Set torpedo type.
            \param torpedoType torpedo type (0=none) */
        void setTorpedoType(int torpedoType);

        /** Set number of torpedoes.
            \param numTorpedoes number of torpedoes */
        void setNumTorpedoes(int numTorpedoes);

        /** Set number of torpedo launchers.
            \param numLaunchers number of launchers */
        void setNumLaunchers(int numLaunchers);

        /** Set number of fighter bays.
            \param numBeams number of fighter bays */
        void setNumBays(int numBays);

        /** Set number of fighters.
            \param numFighters number of fighters */
        void setNumFighters(int numFighters);

        /** Set experience level.
            \return level experience level */
        void setExperienceLevel(int level);

        /** Set beam kill rate.
            \param beamKillRate beam kill rate */
        void setBeamKillRate(int beamKillRate);

        /** Set beam recharge rate.
            \param beamChargeRate beam recharge rate */
        void setBeamChargeRate(int beamChargeRate);

        /** Set torpedo miss rate.
            \param torpMissRate torpedo miss rate */
        void setTorpMissRate(int torpMissRate);

        /** Set torpedo recharge rate.
            \param torpChargeRate torpedo recharge rate */
        void setTorpChargeRate(int torpChargeRate);

        /** Set crew defense rate.
            \param crewDefenseRate crew defense rate (default: 0) */
        void setCrewDefenseRate(int crewDefenseRate);

        /** Set type.
            \param isPlanet true for planet, false for ship */
        void setIsPlanet(bool isPlanet);

        /** Set name.
            \param name name */
        void setName(const String_t& name);

        /** Set role.
            \param role role */
        void setRole(Role role);

        /** Add fighters.
            This is a shortcut for getNumFighters/setNumFighters.
            \param n Fighters to add */
        void addFighters(int n);

        /** Add torpedoes.
            This is a shortcut for getNumTorpedoes/setNumTorpedoes.
            \param n Torpedoes to add */
        void addTorpedoes(int n);

        /** Add fighter bays.
            This is a shortcut for getNumBays/setNumBays.
            \param n Fighter bays to add */
        void addBays(int n);

        /** Add mass.
            This is a shortcut for getMass/setMass.
            \param n Mass to add */
        void addMass(int n);

        /** Remember guessed hull.
            This will guess the hull and update the setHull() field with it.
            This will modify the object in place, so it's a bad idea to call this on VCRs you intend to export,
            but it'll save a few cycles in guessHull() later on.
            \param hulls Hull definitions */
        void setGuessedHull(const game::spec::HullVector_t& hulls);

        /** Check if this could be the specified hull.
            \param hulls Hull definitions
            \param hullId Hull Id to test
            \return true if this unit could be a ship of the the requested hull type */
        bool canBeHull(const game::spec::HullVector_t& hulls, int hullId) const;

        /** Guess this ship's hull.
            Whereas getHull() returns the value from the combat record, which can be missing,
            this will attempt to guess the correct value.
            \param hulls Hull definitions
            \return hull number; zero if ambiguous, impossible, or this is actually a planet */
        int getGuessedHull(const game::spec::HullVector_t& hulls) const;

        /** Get ship picture.
            Whereas getPicture() just returns the value from the combat record,
            this will attempt to resolve the record back into a ship type, and give that ship's picture.
            Thus, it will reflect users' changes.
            \param hulls Hull definitions
            \return picture number; zero if no picture number can be generated */
        int getGuessedShipPicture(const game::spec::HullVector_t& hulls) const;

        /** Guess engine.
            \param engines Engine definitions
            \param pAssumedHull Pointer to hull. Can be null (function will return 0); you can pass h.get(getGuessedHull(h)).
            \param withESB Whether ESB is active in this fight
            \param config Host configuration
            \return Engine number; zero if ambiguous or impossible */
        int getGuessedEngine(const game::spec::EngineVector_t& engines,
                             const game::spec::Hull* pAssumedHull,
                             bool withESB,
                             const game::config::HostConfiguration& config) const;

        /** Get mass for build point computation.
            If the combat mass does NOT include ESB, and hull type is known, replace it by the hull mass.
            Otherwise, returns the combat mass unchanged.
            \param config   Host configuration
            \param shipList Ship list
            \param isPHost  Active ruleset. Can be different from HostVersion in simulation.
            \return mass */
        int getBuildPointMass(const game::config::HostConfiguration& config,
                              const game::spec::ShipList& shipList,
                              bool isPHost) const;

        /** Check for freighter.
            \return true if ship has no weapons. */
        bool isFreighter() const;

        /** Apply classic shield limits.
            Freighters do not have shields. */
        void applyClassicLimits();

        /** Format this object into human-readable form.
            Note that if root, shipList are passed as null, this produces an absolute lo-fi implementation;
            this is mainly for implementation convenience saving the null checks on the caller side.
            \param [in]  teamSettings   Team settings (optional, for viewpoint player)
            \param [in]  root           Root (host configuration, preferences, player names)
            \param [in]  shipList       Unit names
            \param [in]  tx             Translator
            \return result */
        ObjectInfo describe(const TeamSettings* teamSettings, const Root* root, const game::spec::ShipList* shipList, afl::string::Translator& tx) const;

        /** Build a subtitle line for this object.
            The subtitle will include ownership info, type, experience level.
            \param [in]  teamSettings   Team settings (optional, for viewpoint player)
            \param [in]  root           Root (host configuration, preferences, player names)
            \param [in]  shipList       Unit names
            \param [in]  tx             Translator
            \return subtitle */
        String_t getSubtitle(const TeamSettings* teamSettings, const Root& root, const game::spec::ShipList& shipList, afl::string::Translator& tx) const;

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

            // c2ng extensions:
            Role     role;
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

inline game::vcr::Object::Role
game::vcr::Object::getRole() const
{
    return m_data.role;
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
game::vcr::Object::setRole(Role role)
{
    m_data.role = role;
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
