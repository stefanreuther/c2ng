/**
  *  \file server/host/exec.hpp
  *  \brief Execution of Host/Master
  */
#ifndef C2NG_SERVER_HOST_EXEC_HPP
#define C2NG_SERVER_HOST_EXEC_HPP

#include "util/processrunner.hpp"

namespace server { namespace host {

    class Root;

    /** Run host on a game.
        The game must be in state "running" and have game data present.

        This will
        - export game data
        - fetch missing turns
        - run host
        - import game data

        \param runner ProcessRunner
        \param root   Service root
        \param gameId Game to run host for */
    void runHost(util::ProcessRunner& runner, Root& root, int32_t gameId);

    /** Run master on a game.
        The game must not have been mastered/hosted yet
        (a game with masterHasRun set counts as not mastered as far as c2host is concerned).

        This will
        - export game data
        - run master
        - run host
        - import game data

        \param runner ProcessRunner
        \param root   Service root
        \param gameId Game to run master for */
    void runMaster(util::ProcessRunner& runner, Root& root, int32_t gameId);

} }

#endif
