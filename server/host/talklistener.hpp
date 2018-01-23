/**
  *  \file server/host/talklistener.hpp
  *  \brief Interface server::host::TalkListener
  */
#ifndef C2NG_SERVER_HOST_TALKLISTENER_HPP
#define C2NG_SERVER_HOST_TALKLISTENER_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "server/interface/hostgame.hpp"

namespace server { namespace host {

    class Game;

    /** Listener for forum-related actions. */
    class TalkListener : public afl::base::Deletable {
     public:
        /*
         *  Note: these methods traditionally take some game properties as parameter, although those could be queried from the game as well.
         *  However, the caller will already have them available, thus saving us a database query and possible recursion.
         */

        /** Game started.
            Called whenever a game enters an active state (becomes visible to users).
            In this case, a forum shall be created for the game.
            \param game      Game
            \param gameType  Game type (game.getType()) */
        virtual void handleGameStart(Game& game, server::interface::HostGame::Type gameType) = 0;

        /** Game finished.
            Called whenever a game finishes.
            In this case, a forum shall be retired.
            \param game      Game
            \param gameType  Game type (game.getType()) */
        virtual void handleGameEnd(Game& game, server::interface::HostGame::Type gameType) = 0;

        /** Game name changed.
            This may affect the forum name.
            \param game      Game
            \param newName   New name (game.getName()) */
        virtual void handleGameNameChange(Game& game, const String_t& newName) = 0;

        /** Game type changed.
            This may affect the forum.
            \param game      Game
            \param gameState Game state (game.getState())
            \param gameType  Game type (game.getType()) */
        virtual void handleGameTypeChange(Game& game, server::interface::HostGame::State gameState, server::interface::HostGame::Type gameType) = 0;
    };

} }

#endif
