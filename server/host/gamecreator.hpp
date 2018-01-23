/**
  *  \file server/host/gamecreator.hpp
  *  \brief Class server::host::GameCreator
  */
#ifndef C2NG_SERVER_HOST_GAMECREATOR_HPP
#define C2NG_SERVER_HOST_GAMECREATOR_HPP

#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "server/interface/hostgame.hpp"

namespace server { namespace host {

    class Root;
    class TalkListener;

    /** Functions to create games.
        Creating a game is a multi-step process.

        - call createNewGame() to create a game
        - set up the game (initializeGame() or copyGame()), but do not set type/state yet
        - call finishNewGame() to finish and publish the game */
    class GameCreator {
     public:
        /** Constructor.
            \param root Service root */
        explicit GameCreator(Root& root);

        /** Create a new game.
            This will allocate a new game number and create file system content.
            It will not place the game on any lists.
            \return newly-allocated game number */
        int32_t createNewGame();

        /** Initialize a game.
            This will set up defaults for tool/host/master/shiplist and name,
            and create the game slots.
            \param gameId newly-allocated game number */
        void initializeGame(int32_t gameId);

        /** Copy a game.
            For best results, the source game should be preparing, but other states are handled as well.
            The destination game should be newly-allocated (initializeGame() not called).

            This will copy all game slots, most settings, the schedule, tool/host/master/shiplist assignments.
            The name will be set built from the source game's name and a counter.

            \param srcId Source game Id
            \param dstId Newly-allocated destination game Id */
        void copyGame(int32_t srcId, int32_t dstId);

        /** Finish game creation.
            This places the game on the respective lists so it can be found by other commands.
            \param id Game id
            \param state Target game state
            \param type Target game type */
        void finishNewGame(int32_t id, server::interface::HostGame::State state, server::interface::HostGame::Type type);

        /** Pick daytime for a new game.
            In case the schedule for a host does not specify one, pick one which is at a (supposedly) idle hour.
            This is just an approximate, brute-force approach to balance server load.
            \return suggested dayTime value (minutes since midnight). */
        int32_t pickDayTime();

     private:
        Root& m_root;
    };

} }

#endif
