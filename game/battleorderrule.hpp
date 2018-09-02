/**
  *  \file game/battleorderrule.hpp
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

    class BattleOrderRule {
     public:
        static const int UNKNOWN = 1200;

        BattleOrderRule(HostVersion host);

        int get(const game::map::Object& obj) const;
        int get(const game::map::Ship& sh) const;
        int get(const game::map::Planet& pl) const;

        int get(const game::sim::Object& obj) const;
        int get(const game::sim::Ship& sh) const;
        int get(const game::sim::Planet& pl) const;

        int getShipBattleOrder(const String_t& friendlyCode,
                               bool hasWeapons,
                               bool hasEnemy,
                               bool hasKillMission) const;
        int getPlanetBattleOrder(const String_t& friendlyCode,
                                 bool hasDefense) const;

     private:
        HostVersion m_host;
    };

}

#endif
