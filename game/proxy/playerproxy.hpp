/**
  *  \file game/proxy/playerproxy.hpp
  *  \brief Class game::proxy::PlayerProxy
  */
#ifndef C2NG_GAME_PROXY_PLAYERPROXY_HPP
#define C2NG_GAME_PROXY_PLAYERPROXY_HPP

#include "game/player.hpp"
#include "game/playerarray.hpp"
#include "game/playerset.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"

namespace game { namespace proxy {

    /** Synchronous, bidirectional proxy for Player Information.
        @see game::PlayerList */
    class PlayerProxy {
     public:
        /** Constructor.
            \param gameSender Game sender */
        PlayerProxy(util::RequestSender<Session> gameSender);

        /** Get set of all players.
            \see PlayerList::getAllPlayers()
            \param link WaitIndicator object
            \return set of players. Empty if session has no player list. */
        PlayerSet_t getAllPlayers(WaitIndicator& link);

        /** Get name of a player.
            \see PlayerList::getPlayerName()
            \param link WaitIndicator object
            \param id Slot
            \param which Which name to get
            \return name. Empty if parameters out of range or session has no player list. */
        String_t getPlayerName(WaitIndicator& link, int id, Player::Name which);

        /** Get names of all players.
            \param link WaitIndicator object
            \param which Which names to get
            \return array of names. Values for empty or out-of-range indexes are empty. */
        PlayerArray<String_t> getPlayerNames(WaitIndicator& link, Player::Name which);

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
