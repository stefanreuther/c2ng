/**
  *  \file game/spec/torpedolauncher.hpp
  *  \brief Class game::spec::TorpedoLauncher
  */
#ifndef C2NG_GAME_SPEC_TORPEDOLAUNCHER_HPP
#define C2NG_GAME_SPEC_TORPEDOLAUNCHER_HPP

#include "game/spec/weapon.hpp"
#include "game/spec/cost.hpp"

namespace game { namespace spec {

    /** A torpedo launcher.
        This class only holds data which it does not interpret or limit.

        This is the primary data class for torpedo systems;
        class Torpedo can be used to access a single torpedo instead of the launcher. */
    class TorpedoLauncher : public Weapon {
     public:
        /** Constructor.
            \param torpedo Id */
        explicit TorpedoLauncher(int id);

        /** Get torpedo cost.
            \return cost */
        Cost& torpedoCost();

        /** Get torpedo cost.
            \return cost
            \overload */
        const Cost& torpedoCost() const;

     private:
        Cost m_torpedoCost;
    };

} }

#endif
