/**
  *  \file game/battleorderrule.hpp
  *  \brief Class game::BattleOrderRule
  */
#ifndef C2NG_GAME_BATTLEORDERRULE_HPP
#define C2NG_GAME_BATTLEORDERRULE_HPP

#include "game/hostversion.hpp"
#include "game/map/object.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/sim/object.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/ship.hpp"

namespace game {

    /** Battle Order Rule.

        This class contains functions to determine the battle order of units, using the appropriate host-dependant rules.
        It can be used with regular (game::map::Object) and simulator (game::sim::Object) units.

        Battle order is a value between 0 and 1015 (Host) resp. -99 and 1004 (PHost), where lower numbers mean fight first.
        For units for which we do not know a battle order, or which do not have one according to the host version,
        we return UNKNOWN which sorts after all battle orders. */
    class BattleOrderRule {
     public:
        /** Marker for unknown battle order.
            Larger than all regular known battle orders. */
        static const int UNKNOWN = 1200;

        /** Constructor.
            \param host Host version to work for */
        BattleOrderRule(HostVersion host);


        /*
         *  Map Object
         */

        /** Get battle order for map object.
            \param obj Object
            \return battle order */
        int get(const game::map::Object& obj) const;

        /** Get battle order for ship.
            \param sh Ship
            \return battle order */
        int get(const game::map::Ship& sh) const;

        /** Get battle order for planet.
            \param pl Planet
            \return battle order */
        int get(const game::map::Planet& pl) const;


        /*
         *  Sim Object
         */

        /** Get battle order for simulator object.
            \param obj Object
            \return battle order */
        int get(const game::sim::Object& obj) const;

        /** Get battle order for simulator ship.
            \param sh Ship
            \return battle order */
        int get(const game::sim::Ship& sh) const;

        /** Get battle order for simulator planet.
            \param pl Planet
            \return battle order */
        int get(const game::sim::Planet& pl) const;


        /*
         *  Manual
         */

        /** Get battle order for ship, given its parameters.
            \param friendlyCode Friendly Code
            \param hasWeapons   true if ship has any weapons
            \param hasEnemy     true if ship has a nonzero primary enemy
            \param hasKillMission true if ship has mission Kill
            \param hasFuel      true if ship has fuel
            \return battle order */
        int getShipBattleOrder(const String_t& friendlyCode,
                               bool hasWeapons,
                               bool hasEnemy,
                               bool hasKillMission,
                               bool hasFuel) const;

        /** Get battle order for planet, given its parameters.
            \param friendlyCode Friendly Code
            \param hasDefense   true if planet has defense posts
            \return battle order */
        int getPlanetBattleOrder(const String_t& friendlyCode,
                                 bool hasDefense) const;

     private:
        HostVersion m_host;
    };

}

#endif
