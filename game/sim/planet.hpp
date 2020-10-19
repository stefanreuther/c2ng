/**
  *  \file game/sim/planet.hpp
  *  \brief Class game::sim::Planet
  */
#ifndef C2NG_GAME_SIM_PLANET_HPP
#define C2NG_GAME_SIM_PLANET_HPP

#include "game/sim/object.hpp"

namespace game { namespace sim {

    /** Planet for simulation.
        All properties are freely editable. */
    class Planet : public Object {
     public:
        /** Maximum number of torpedo types supported. */
        static const int NUM_TORPEDO_TYPES = 10;

        /** Default constructor. */
        Planet();

        /** Destructor. */
        ~Planet();

        /** Get number of planetary defense posts.
            \return number */
        int getDefense() const;

        /** Set number of planetary defense posts.
            \param defense number */
        void setDefense(int defense);

        /** Get number of starbase defense posts.
            \return number */
        int getBaseDefense() const;

        /** Set number of starbase defense posts.
            \param defense number */
        void setBaseDefense(int baseDefense);

        /** Get starbase beam tech level.
            Zero means no starbase.
            \return tech level */
        int getBaseBeamTech() const;

        /** Set starbase beam tech level.
            Zero means no starbase.
            \param beamTech tech level */
        void setBaseBeamTech(int beamTech);

        /** Get starbase torpedo tech level.
            \return tech level */
        int getBaseTorpedoTech() const;

        /** Set starbase torpedo tech level.
            \param torpTech tech level */
        void setBaseTorpedoTech(int torpTech);

        /** Get number of starbase fighters.
            \return number */
        int getNumBaseFighters() const;

        /** Set number of starbase fighters.
            \param baseFighters number */
        void setNumBaseFighters(int baseFighters);

        /** Get number of starbase torpedoes of a given type.
            \param type Type [1,NUM_TORPEDO_TYPES]
            \return Number of torpedoes; 0 if type is out-of-range */
        int getNumBaseTorpedoes(int type) const;

        /** Set number of starbase torpedoes of a given type.
            \param type Type [1,NUM_TORPEDO_TYPES]. If type is out-of-range, the call is ignored.
            \param amount Amount */
        void setNumBaseTorpedoes(int type, int amount);

        // // FIXME!!!!!!!!!!!!!!!!!1111
        int getBaseDamage() const
            { return 0; }
        void setBaseDamage(int)
            { }

        /** Check presence of a starbase.
            This is a shortcut to the getBaseBeamTech() call.
            \return true if starbase is present */
        bool hasBase() const;

        /** Get total number of starbase torpedoes as one type.
            This function computes the effective torpedo count from all stored torpedoes.
            This is used to get one torpedo count from a mixed set of torpedoes for fighting.
            \param type desired type
            \param shipList ship list containing torpedo definitions
            \return combined torpedo count */
        int32_t getNumBaseTorpedoesAsType(int type, const game::spec::ShipList& shipList) const;

        // Object:
        virtual bool hasImpliedAbility(Ability which, const Configuration& opts, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const;

     private:
        int  m_defense;                          // ex defense
        int  m_baseDefense;                      // ex base_defense
        int  m_beamTech;                         // ex beam_tech             /* 0 = no base */
        int  m_torpedoTech;                      // ex torp_tech
        int  m_baseFighters;                     // ex base_fighters
        int  m_baseTorpedoes[NUM_TORPEDO_TYPES]; // ex base_torps
    };

} }

#endif
