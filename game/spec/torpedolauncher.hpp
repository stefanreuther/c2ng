/**
  *  \file game/spec/torpedolauncher.hpp
  */
#ifndef C2NG_GAME_SPEC_TORPEDOLAUNCHER_HPP
#define C2NG_GAME_SPEC_TORPEDOLAUNCHER_HPP

#include "game/spec/weapon.hpp"
#include "game/spec/cost.hpp"

namespace game { namespace spec {

    class TorpedoLauncher : public Weapon {
     public:
        explicit TorpedoLauncher(int id);

        Cost& torpedoCost();
        const Cost& torpedoCost() const;

     private:
        Cost m_torpedoCost;
    };

} }

#endif
