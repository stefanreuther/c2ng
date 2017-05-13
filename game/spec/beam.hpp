/**
  *  \file game/spec/beam.hpp
  *  \brief Class game::spec::Beam
  */
#ifndef C2NG_GAME_SPEC_BEAM_HPP
#define C2NG_GAME_SPEC_BEAM_HPP

#include "game/spec/weapon.hpp"

namespace game { namespace spec {

    /** A beam weapon.
        This class only holds data which it does not interpret or limit. */
    class Beam : public Weapon {
     public:
        /** Constructor.
            \param beam Id */
        explicit Beam(int id);
    };

} }

#endif
