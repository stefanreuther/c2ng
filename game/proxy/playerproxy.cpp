/**
  *  \file game/proxy/playerproxy.cpp
  *  \brief Class game::proxy::PlayerProxy
  */

#include "game/proxy/playerproxy.hpp"
#include "game/playerlist.hpp"
#include "game/root.hpp"

namespace {
    game::PlayerList* getPlayerList(game::Session& s)
    {
        if (game::Root* pRoot = s.getRoot().get()) {
            return &pRoot->playerList();
        } else {
            return 0;
        }
    }
}


game::proxy::PlayerProxy::PlayerProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

game::PlayerSet_t
game::proxy::PlayerProxy::getAllPlayers(WaitIndicator& link)
{
    PlayerSet_t result;

    class Query : public util::Request<Session> {
     public:
        Query(PlayerSet_t& result)
            : m_result(result)
            { }
        virtual void handle(Session& s)
            {
                if (PlayerList* p = getPlayerList(s)) {
                    m_result = p->getAllPlayers();
                }
            }
     private:
        PlayerSet_t& m_result;
    };

    Query q(result);
    link.call(m_gameSender, q);

    return result;
}

String_t
game::proxy::PlayerProxy::getPlayerName(WaitIndicator& link, int id, Player::Name which)
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
                    m_result = p->getPlayerName(m_id, m_which, s.translator());
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
game::proxy::PlayerProxy::getPlayerNames(WaitIndicator& link, Player::Name which)
{
    PlayerArray<String_t> result;

    class Query : public util::Request<Session> {
     public:
        Query(Player::Name which, PlayerArray<String_t>& result)
            : m_which(which),
              m_result(result)
            { }
        virtual void handle(Session& s)
            {
                if (PlayerList* p = getPlayerList(s)) {
                    for (int i = 0; i <= MAX_PLAYERS; ++i) {
                        if (Player* pl = p->get(i)) {
                            m_result.set(i, pl->getName(m_which, s.translator()));
                        }
                    }
                }
            }
     private:
        Player::Name m_which;
        PlayerArray<String_t>& m_result;
    };

    Query q(which, result);
    link.call(m_gameSender, q);

    return result;
}
