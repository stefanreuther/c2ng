/**
  *  \file server/host/actions.hpp
  *  \brief Actions on games
  */
#ifndef C2NG_SERVER_HOST_ACTIONS_HPP
#define C2NG_SERVER_HOST_ACTIONS_HPP

#include "afl/base/types.hpp"
#include "server/host/game.hpp"
#include "server/interface/filebase.hpp"

namespace server { namespace host {

    class Root;

    /** Drop slot if it is dead.

        \param game Game to work on
        \param slot Slot to check

        \retval true Slot was dead and has been removed from the game
        \retval false Slot has nonzero score and was not removed

        \pre
        - host run complete, importGameData() has been run
        - database lock acquired
        - slot is not being played */
    bool dropSlotIfDead(Game& game, int slot);

    /** Kick inactive players.
        This implements the Configuration::numMissedTurnsForKick option.

        \param root Service root
        \param gameId Game Id

        \pre
        - host run complete, importGameData() has been run
        - database lock acquired */
    void processInactivityKicks(Root& root, int32_t gameId);

    /** Import file history for one turn.
        Updates \c fileHistory with the correct file name lists.
        \param hostFile    Host filer
        \param gameDir     Directory within hostFile
        \param turnNumber  Turn number
        \param fileHistory Database node (Game(x).turn(turnNumber).files() */
    void importFileHistory(server::interface::FileBase& hostFile,
                           const String_t& gameDir,
                           const int turnNumber,
                           Game::TurnFiles fileHistory);

    /** Import file history for all turns.
        This function is used to catch up a game that has been hosted with importFileHistory() not being called.

        The newly-created history will be an estimate:
        it uses the currently-published files to determine which files to publish.
        If a file was public then but is not now, it will still be shown.
        If a file was not public then, but is now, it will be shown.
        This function is therefore only for transition periods.

        \param hostFile Host filer
        \param game     Game */
    void importAllFileHistory(server::interface::FileBase& hostFile, Game& game);

} }

#endif
