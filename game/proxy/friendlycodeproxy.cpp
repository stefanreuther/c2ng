/**
  *  \file game/proxy/friendlycodeproxy.cpp
  *  \brief Class game::proxy::FriendlyCodeProxy
  */

#include "game/proxy/friendlycodeproxy.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "util/request.hpp"

game::proxy::FriendlyCodeProxy::FriendlyCodeProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

String_t
game::proxy::FriendlyCodeProxy::generateRandomCode(WaitIndicator& link)
{
    class Query : public util::Request<game::Session> {
     public:
        Query(String_t& result)
            : m_result(result)
            { }
        virtual void handle(Session& session)
            {
                Root* pRoot = session.getRoot().get();
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
