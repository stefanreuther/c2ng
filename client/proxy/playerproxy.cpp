/**
  *  \file client/proxy/playerproxy.cpp
  */

#include "client/proxy/playerproxy.hpp"
#include "game/playerlist.hpp"
#include "game/root.hpp"

using game::Player;
using game::PlayerList;
using game::Session;

namespace {
    PlayerList* getPlayerList(Session& s)
    {
        if (game::Root* pRoot = s.getRoot().get()) {
            return &pRoot->playerList();
        } else {
            return 0;
        }
    }
}


client::proxy::PlayerProxy::PlayerProxy(util::RequestSender<game::Session> gameSender)
    : m_gameSender(gameSender)
{ }

game::PlayerSet_t
client::proxy::PlayerProxy::getAllPlayers(Downlink& link)
{
    game::PlayerSet_t result;

    class Query : public util::Request<Session> {
     public:
        Query(game::PlayerSet_t& result)
            : m_result(result)
            { }
        virtual void handle(Session& s)
            {
                if (PlayerList* p = getPlayerList(s)) {
                    m_result = p->getAllPlayers();
                }
            }
     private:
        game::PlayerSet_t& m_result;
    };

    Query q(result);
    link.call(m_gameSender, q);

    return result;
}

String_t
client::proxy::PlayerProxy::getPlayerName(Downlink& link, int id, game::Player::Name which)
{
    String_t result;

    class Query : public util::Request<Session> {
     public:
        Query(int id, Player::Name which, String_t& result)
            : m_id(id), m_which(which),
              m_result(result)
            { }
        virtual void handle(Session& s)
            {
                if (PlayerList* p = getPlayerList(s)) {
                    m_result = p->getPlayerName(m_id, m_which);
                }
            }
     private:
        int m_id;
        Player::Name m_which;
        String_t& m_result;
    };

    Query q(id, which, result);
    link.call(m_gameSender, q);

    return result;
}

game::PlayerArray<String_t>
client::proxy::PlayerProxy::getPlayerNames(Downlink& link, game::Player::Name which)
{
    game::PlayerArray<String_t> result;

    class Query : public util::Request<Session> {
     public:
        Query(Player::Name which, game::PlayerArray<String_t>& result)
            : m_which(which),
              m_result(result)
            { }
        virtual void handle(Session& s)
            {
                if (PlayerList* p = getPlayerList(s)) {
                    for (int i = 0; i <= game::MAX_PLAYERS; ++i) {
                        if (Player* pl = p->get(i)) {
                            m_result.set(i, pl->getName(m_which));
                        }
                    }
                }
            }
     private:
        Player::Name m_which;
        game::PlayerArray<String_t>& m_result;
    };

    Query q(which, result);
    link.call(m_gameSender, q);

    return result;
}
