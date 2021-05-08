/**
  *  \file game/sim/sessionextra.hpp
  *  \brief Game/Simulator Session
  */
#ifndef C2NG_GAME_SIM_SESSIONEXTRA_HPP
#define C2NG_GAME_SIM_SESSIONEXTRA_HPP

#include "game/sim/session.hpp"
#include "game/session.hpp"
#include "afl/base/ref.hpp"

namespace game { namespace sim {

    /** Get simulator session for a game session.
        Creates the session or returns the previously created instance.
        The simulator session is managed as an extra (game::Session::extra()).

        The simulator session automatically receives a GameInterface implementation
        to connect it to the game session. */
    afl::base::Ref<Session> getSimulatorSession(game::Session& session);

} }

#endif
