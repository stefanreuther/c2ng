/**
  *  \file client/proxy/configurationproxy.cpp
  */

#include "client/proxy/configurationproxy.hpp"
#include "game/root.hpp"

using game::Session;
using game::Root;

client::proxy::ConfigurationProxy::ConfigurationProxy(util::RequestSender<game::Session> gameSender)
    : m_gameSender(gameSender)
{ }

util::NumberFormatter
client::proxy::ConfigurationProxy::getNumberFormatter(Downlink& link)
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
