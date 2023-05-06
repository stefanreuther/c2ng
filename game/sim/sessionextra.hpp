/**
  *  \file game/sim/sessionextra.hpp
  *  \brief Game/Simulator Session
  */
#ifndef C2NG_GAME_SIM_SESSIONEXTRA_HPP
#define C2NG_GAME_SIM_SESSIONEXTRA_HPP

#include "afl/base/ref.hpp"
#include "game/session.hpp"
#include "game/sim/session.hpp"

namespace game { namespace sim {

    /** Get simulator session for a game session.
        Creates the session or returns the previously created instance.
        The simulator session is managed as an extra (game::Session::extra()).

        The simulator session automatically receives a GameInterface implementation
        to connect it to the game session.

        @param session Game session
        @return Simulator session */
    afl::base::Ref<Session> getSimulatorSession(game::Session& session);

    /** Initialize simulator session.
        Call this after a new game has loaded into the game session.

        @param session Game session */
    void initSimulatorSession(game::Session& session);

} }

#endif
