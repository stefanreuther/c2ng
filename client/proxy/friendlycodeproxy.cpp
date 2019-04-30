/**
  *  \file client/proxy/friendlycodeproxy.cpp
  */

#include "client/proxy/friendlycodeproxy.hpp"
#include "util/request.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"

client::proxy::FriendlyCodeProxy::FriendlyCodeProxy(util::RequestSender<game::Session> gameSender)
    : m_gameSender(gameSender)
{ }

String_t
client::proxy::FriendlyCodeProxy::generateRandomCode(Downlink& link)
{
    class Query : public util::Request<game::Session> {
     public:
        Query(String_t& result)
            : m_result(result)
            { }
        virtual void handle(game::Session& session)
            {
                game::Root* pRoot = session.getRoot().get();
                game::spec::ShipList* pShipList = session.getShipList().get();
                if (pRoot != 0 && pShipList != 0) {
                    m_result = pShipList->friendlyCodes().generateRandomCode(session.rng(), pRoot->hostVersion());
                }
            }
     private:
        String_t& m_result;
    };

    String_t result;
    Query q(result);
    link.call(m_gameSender, q);

    return result;
}
