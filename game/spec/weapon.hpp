/**
  *  \file game/spec/weapon.hpp
  */
#ifndef C2NG_GAME_SPEC_WEAPON_HPP
#define C2NG_GAME_SPEC_WEAPON_HPP

#include "game/spec/component.hpp"

namespace game { namespace spec {

    class Weapon : public Component {
     public:
        Weapon(ComponentNameProvider::Type type, int id);

        int getKillPower() const;
        void setKillPower(int killPower);

        int getDamagePower() const;
        void setDamagePower(int damagePower);

     private:
        int m_killPower;
        int m_damagePower;
    };

} }

#endif
