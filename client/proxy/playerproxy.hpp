/**
  *  \file client/proxy/playerproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_PLAYERPROXY_HPP
#define C2NG_CLIENT_PROXY_PLAYERPROXY_HPP

#include "client/downlink.hpp"
#include "game/player.hpp"
#include "game/playerarray.hpp"
#include "game/playerset.hpp"
#include "game/session.hpp"

namespace client { namespace proxy {

    /** Synchronous, bidirectional proxy for Player Information.
        @see game::PlayerList */
    class PlayerProxy {
     public:
        /** Constructor.
            \param gameSender Game sender */
        PlayerProxy(util::RequestSender<game::Session> gameSender);

        /** Get set of all players.
            \see game::PlayerList::getAllPlayers()
            \param link Downlink object
            \return set of players. Empty if session has no player list. */
        game::PlayerSet_t getAllPlayers(Downlink& link);

        /** Get name of a player.
            \see game::PlayerList::getPlayerName()
            \param link Downlink object
            \param id Slot
            \param which Which name to get
            \return name. Empty if parameters out of range or session has no player list. */
        String_t getPlayerName(Downlink& link, int id, game::Player::Name which);

        /** Get names of all players.
            \param link Downlink object
            \param which Which names to get
            \return array of names. Values for empty or out-of-range indexes are empty. */
        game::PlayerArray<String_t> getPlayerNames(Downlink& link, game::Player::Name which);

     private:
        util::RequestSender<game::Session> m_gameSender;
    };

} }

#endif
