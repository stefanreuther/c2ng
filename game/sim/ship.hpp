/**
  *  \file game/sim/ship.hpp
  *  \brief Class game::sim::Ship
  */
#ifndef C2NG_GAME_SIM_SHIP_HPP
#define C2NG_GAME_SIM_SHIP_HPP

#include "afl/string/translator.hpp"
#include "game/sim/object.hpp"
#include "game/spec/shiplist.hpp"
#include "util/range.hpp"

namespace game { namespace sim {

    /** Ship for simulation.
        All properties are freely editable. */
    class Ship : public Object {
     public:
        /** Aggressiveness values. Values between 1 and 12 correspond to primary enemy settings.
            FIXME: how do we deal with >11 players? */
        /// "Kill" mission.
        static const int agg_Kill    = -1;
        /// Passive unit.
        static const int agg_Passive = 0;
        /// Unit has no fuel.
        static const int agg_NoFuel  = 13;

        /** Default constructor. */
        Ship();

        /** Destructor. */
        ~Ship();

        /** Get crew.
            \return crew */
        int getCrew() const;

        /** Set crew.
            \param crew Crew */
        void setCrew(int crew);

        /** Get hull type.
            \return hull number (can be 0 for custom ships). */
        int getHullType() const;

        /** Set hull type.
            This also sets other properties as appropriate for the hull type.
            \param hullType hull number (0 for custom ship)
            \param shipList ship list */
        void setHullType(int hullType, const game::spec::ShipList& shipList);

        /** Set hull type only.
            Unlike setHullType, does not update other properties.
            \param hullType hull number (0 for custom ship) */
        void setHullTypeOnly(int hullType);

        /** Get mass.
            \return mass */
        int getMass() const;

        /** Set mass.
            Should only be used for custom ships; for normal ships, it is maintained by setHullType().
            \param mass Mass */
        void setMass(int mass);

        /** Get beam type.
            \return beam type */
        int getBeamType() const;

        /** Get beam type.
            \param beamType beam type */
        void setBeamType(int beamType);

        /** Get number of beams.
            \return number of beams */
        int getNumBeams() const;

        /** Set number of beams.
            \param numBeams number of beams */
        void setNumBeams(int numBeams);

        /** Get torpedo type.
            \return torpedo type */
        int getTorpedoType() const;

        /** Set torpedo type.
            \param torpedoType torpedo type */
        void setTorpedoType(int torpedoType);

        /** Get number of torpedo launchers.
            \return number of torpedo launchers */
        int getNumLaunchers() const;

        /** Set number of torpedo launchers.
            \param numLaunchers number of torpedo launchers */
        void setNumLaunchers(int numLaunchers);

        /** Get number of fighter bays.
            \return number of fighter bays */
        int getNumBays() const;

        /** Set number of fighter bays.
            \param numBays number of fighter bays */
        void setNumBays(int numBays);

        /** Get number of torpedoes/fighters.
            \return number */
        int getAmmo() const;

        /** Set number of torpedoes/fighters.
            \param ammo number */
        void setAmmo(int ammo);

        /** Get engine type.
            \return engine type */
        int getEngineType() const;

        /** Set engine type.
            \param engineType engine type */
        void setEngineType(int engineType);

        /** Get aggressiveness.
            \return aggressiveness */
        int getAggressiveness() const;

        /** Set aggressiveness.
            \param aggressiveness aggressiveness */
        void setAggressiveness(int aggressiveness);

        /** Get Id for intercept-attack.
            \return Id */
        int getInterceptId() const;

        /** Set Id for intercept-attack.
            \param id Id */
        void setInterceptId(int id);

        /** Check for default name.
            \param tx Translator
            \return true if this ship has a default name */
        bool hasDefaultName(afl::string::Translator& tx) const;

        /** Set default name.
            A ship's default name depends on its Id number only.
            If you change a ship's Id, call hasDefaultName() before the change;
            if it had a default name, call setDefaultName() afterwards to restore it.
            \param tx Translator */
        void setDefaultName(afl::string::Translator& tx);

        /** Check for custom ship.
            A custom ship is not limited by a hull definition.
            \return true for custom ships */
        bool isCustomShip() const;

        /** Get range of number of beams.
            \param shipList Ship List
            \return Range */
        util::Range<int> getNumBeamsRange(const game::spec::ShipList& shipList) const;

        /** Get range of number of torpedo launchers.
            \param shipList Ship List
            \return Range */
        util::Range<int> getNumLaunchersRange(const game::spec::ShipList& shipList) const;

        /** Get range of fighter bays.
            \param shipList Ship List
            \return Range */
        util::Range<int> getNumBaysRange(const game::spec::ShipList& shipList) const;

        /** Check whether this ship matches a ship list.
            \param shipList ship list
            \return true if the ship is legal according to the ship list, that is,
            its properties match the ship list's limits.
            A custom ship is always legal. */
        bool isMatchingShipList(const game::spec::ShipList& shipList) const;

        /** Check for implied hull function.
            This function check any basic function, not just those mapped to simulator abilities.
            \param basicFunctionId Function to check
            \param shipList        Ship list
            \param config          Host configuration
            \return true if this ship's hull has this implied function */
        bool hasImpliedFunction(int basicFunctionId, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const;

        // Object:
        virtual bool hasImpliedAbility(Ability which, const Configuration& opts, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const;

        /** Check for primary enemy.
            \param agg Aggressiveness setting
            \return true if aggressiveness setting corresponds to a primary enemy */
        static bool isPrimaryEnemy(int agg);

     private:
        int m_crew;             // ex crew
        int m_hullType;         // ex hull
        int m_mass;             // ex mass      /* custom ships */
        int m_beamType;         // ex beam_type
        int m_numBeams;         // ex beam_count
        int m_torpedoType;      // ex torp_type
        int m_numLaunchers;     // ex torp_lcount
        int m_numBays;          // ex bay_count
        int m_ammo;             // ex ammo
        int m_engineType;       // ex engine_type
        int m_aggressiveness;   // ex aggressiveness
        int m_interceptId;      // ex intercept_id
    };

} }

#endif
