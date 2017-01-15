/**
  *  \file game/sim/planet.hpp
  */
#ifndef C2NG_GAME_SIM_PLANET_HPP
#define C2NG_GAME_SIM_PLANET_HPP

#include "game/sim/object.hpp"

namespace game { namespace sim {

    class Planet : public Object {
     public:
        static const int NUM_TORPEDO_TYPES = 10;

        Planet();
        ~Planet();

        int  getDefense() const;
        void setDefense(int defense);

        int  getBaseDefense() const;
        void setBaseDefense(int baseDefense);

        int  getBaseBeamTech() const;
        void setBaseBeamTech(int beamTech);

        int  getBaseTorpedoTech() const;
        void setBaseTorpedoTech(int torpTech);

        int  getNumBaseFighters() const;
        void setNumBaseFighters(int baseFighters);

        int  getNumBaseTorpedoes(int type) const;
        void setNumBaseTorpedoes(int type, int amount);

        // // FIXME!!!!!!!!!!!!!!!!!1111
        // int  getBaseDamage() const
        //     { return 0; }
        // void setBaseDamage(int)
        //     { }

        bool hasBase() const;
        int32_t getNumBaseTorpedoesAsType(int type, const game::spec::ShipList& shipList) const;

        virtual bool hasImpliedAbility(Ability which, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const;

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
