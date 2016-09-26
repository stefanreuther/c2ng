/**
  *  \file game/spec/beam.hpp
  */
#ifndef C2NG_GAME_SPEC_BEAM_HPP
#define C2NG_GAME_SPEC_BEAM_HPP

#include "game/spec/weapon.hpp"

namespace game { namespace spec {

    class Beam : public Weapon {
     public:
        explicit Beam(int id);
    };

} }

#endif
