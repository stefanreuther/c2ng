/**
  *  \file game/proxy/configurationproxy.cpp
  *  \brief Class game::proxy::ConfigurationProxy
  */

#include "game/proxy/configurationproxy.hpp"
#include "game/root.hpp"

game::proxy::ConfigurationProxy::ConfigurationProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

util::NumberFormatter
game::proxy::ConfigurationProxy::getNumberFormatter(WaitIndicator& link)
{
    class Query : public util::Request<Session> {
     public:
        Query(util::NumberFormatter& result)
            : m_result(result)
            { }

        virtual void handle(Session& session)
            {
                if (Root* pRoot = session.getRoot().get()) {
                    m_result = pRoot->userConfiguration().getNumberFormatter();
                }
            }
     private:
        util::NumberFormatter& m_result;
    };

    // Initialize with default
    util::NumberFormatter result(true, false);

    Query q(result);
    link.call(m_gameSender, q);

    return result;
}
