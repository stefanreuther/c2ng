/**
  *  \file game/sim/ship.hpp
  */
#ifndef C2NG_GAME_SIM_SHIP_HPP
#define C2NG_GAME_SIM_SHIP_HPP

#include "game/sim/object.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace sim {

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

        Ship();
        ~Ship();

        int  getCrew() const;
        void setCrew(int crew);

        int  getHullType() const;
        void setHullType(int hullType, const game::spec::ShipList& shipList);
        void setHullTypeOnly(int hullType);

        int  getMass() const;
        void setMass(int mass);

        int  getBeamType() const;
        void setBeamType(int beamType);
        int  getNumBeams() const;
        void setNumBeams(int numBeams);

        int  getTorpedoType() const;
        void setTorpedoType(int torpedoType);
        int  getNumLaunchers() const;
        void setNumLaunchers(int numLaunchers);

        int  getNumBays() const;
        void setNumBays(int numBays);

        int  getAmmo() const;
        void setAmmo(int ammo);

        int  getEngineType() const;
        void setEngineType(int engineType);

        int  getAggressiveness() const;
        void setAggressiveness(int aggressiveness);

        int  getInterceptId() const;
        void setInterceptId(int id);

        bool hasDefaultName(afl::string::Translator& tx) const;
        void setDefaultName(afl::string::Translator& tx);

        bool isCustomShip() const;
        bool isMatchingShipList(const game::spec::ShipList& shipList) const;

        virtual bool hasImpliedAbility(Ability which, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const;

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
