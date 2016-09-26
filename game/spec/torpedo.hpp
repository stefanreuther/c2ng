/**
  *  \file game/spec/torpedo.hpp
  */
#ifndef C2NG_GAME_SPEC_TORPEDO_HPP
#define C2NG_GAME_SPEC_TORPEDO_HPP

#include "game/spec/weapon.hpp"

namespace game { namespace spec {

    class TorpedoLauncher;

    class Torpedo : public Weapon {
     public:
        Torpedo(const TorpedoLauncher& launcher);
    };

} }

#endif
