/**
  *  \file game/spec/torpedo.hpp
  *  \brief Class game::spec::Torpedo
  */
#ifndef C2NG_GAME_SPEC_TORPEDO_HPP
#define C2NG_GAME_SPEC_TORPEDO_HPP

#include "game/spec/weapon.hpp"

namespace game { namespace spec {

    class TorpedoLauncher;

    /** A torpedo launcher.
        This class only holds data which it does not interpret or limit.

        This class is constructed from a TorpedoLauncher which is the primary data class for torpedo systems. */
    class Torpedo : public Weapon {
     public:
        /** Constructor.
            \param launcher Launcher object; data is copied from it */
        explicit Torpedo(const TorpedoLauncher& launcher);
    };

} }

#endif
