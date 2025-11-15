/**
  *  \file game/spec/weapon.hpp
  *  \brief Class game::spec::Weapon
  */
#ifndef C2NG_GAME_SPEC_WEAPON_HPP
#define C2NG_GAME_SPEC_WEAPON_HPP

#include "game/spec/component.hpp"
#include "game/hostversion.hpp"

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

        /** Check for death ray.
            \param host Host Version
            \return true if this is a death ray */
        bool isDeathRay(const HostVersion& host) const;

     private:
        int m_killPower;
        int m_damagePower;
    };

} }

// Get kill power.
inline int
game::spec::Weapon::getKillPower() const
{
    return m_killPower;
}

// Set kill power.
inline void
game::spec::Weapon::setKillPower(int killPower)
{
    m_killPower = killPower;
}

// Get damage power.
inline int
game::spec::Weapon::getDamagePower() const
{
    return m_damagePower;
}

// Set damage power.
inline void
game::spec::Weapon::setDamagePower(int damagePower)
{
    m_damagePower = damagePower;
}

#endif
