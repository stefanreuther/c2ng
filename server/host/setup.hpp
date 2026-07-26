/**
  *  \file server/host/setup.hpp
  *  \brief Host directory setup
  */
#ifndef C2NG_SERVER_HOST_SETUP_HPP
#define C2NG_SERVER_HOST_SETUP_HPP

#include "server/host/game.hpp"
#include "server/host/root.hpp"

namespace server { namespace host {

    /** Pre-host setup.
        This function is invoked before running master.
        Works together with the runmaster.sh script to produce a game directory.
        The work split is rather arbitrary.

        As of July 2026, this function's main focus is to prepare a PlayerRace option.
        Most file copying is done in runmaster.sh.

        @param root    Service root
        @param game    Game */
    void performPreHostSetup(Root& root, Game& game);

} }

#endif
