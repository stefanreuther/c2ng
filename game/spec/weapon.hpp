/**
  *  \file game/spec/weapon.hpp
  *  \brief Class game::spec::Weapon
  */
#ifndef C2NG_GAME_SPEC_WEAPON_HPP
#define C2NG_GAME_SPEC_WEAPON_HPP

#include "game/spec/component.hpp"

namespace game { namespace spec {

    /** A weapon.
        This is the common base class for weapon components (beam, torpedo).
        It only holds data which it does not interpret or limit. */
    class Weapon : public Component {
     public:
        /** Constructor.
            \param type Weapon type
            \param id Id */
        Weapon(ComponentNameProvider::Type type, int id);

        /** Get kill power.
            \return kill power */
        int getKillPower() const;

        /** Set kill power.
            \param killPower kill power */
        void setKillPower(int killPower);

        /** Get damage power.
            \return damage power */
        int getDamagePower() const;

        /** Set damage power.
            \param damagePower damage power */
        void setDamagePower(int damagePower);

     private:
        int m_killPower;
        int m_damagePower;
    };

} }

#endif
