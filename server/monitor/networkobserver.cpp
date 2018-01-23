/**
  *  \file server/monitor/networkobserver.cpp
  *  \brief Class server::monitor::NetworkObserver
  */

#include "server/monitor/networkobserver.hpp"
#include "afl/async/controller.hpp"
#include "afl/async/receiveoperation.hpp"
#include "afl/async/sendoperation.hpp"

namespace {
    const afl::sys::Timeout_t NETWORK_TIMEOUT = 10000;

    bool callServer(afl::net::NetworkStack& net, afl::net::Name name, String_t request, String_t& reply, bool useShutdown)
    {
        // ex planetscentral/monitor.cc:callServer
        // Clear reply, just in case
        reply.clear();

        // Special case: if host is 0.0.0.0, connect to localhost
        if (name.getName().find_first_not_of("0.") == String_t::npos) {
            name.setName("127.0.0.1");
        }

        // Connect
        try {
            afl::base::Ref<afl::net::Socket> sock = net.connect(name, NETWORK_TIMEOUT);

            // Send data
            afl::async::Controller ctl;
            afl::async::SendOperation sendOp(afl::string::toBytes(request));
            if (!sock->send(ctl, sendOp, NETWORK_TIMEOUT)) {
                // FIXME: report broken, not down?
                return false;
            }
            if (useShutdown) {
                sock->closeSend();
            }

            // Read reply
            const size_t LIMIT = 4096;
            while (reply.size() < LIMIT) {
                uint8_t buffer[LIMIT];
                afl::async::ReceiveOperation recvOp(buffer);
                if (!sock->receive(ctl, recvOp, NETWORK_TIMEOUT)) {
                    // FIXME: report broken, not down?
                    return false;
                }
                if (recvOp.getReceivedBytes().empty()) {
                    break;
                }
                reply += afl::string::fromBytes(recvOp.getReceivedBytes());
            }
            return true;
        }
        catch (std::exception& /*e*/) {
            return false;
        }
    }
}


// Constructor.
server::monitor::NetworkObserver::NetworkObserver(String_t name,
                                                  String_t identifier,
                                                  Flavor flavor,
                                                  afl::net::NetworkStack& net,
                                                  afl::net::Name defaultAddress)
    : StatusObserver(),
      m_name(name),
      m_identifier(identifier),
      m_flavor(flavor),
      m_networkStack(net),
      m_address(defaultAddress)
{ }

// Destructor.
server::monitor::NetworkObserver::~NetworkObserver()
{ }

// Get user-readable name of service.
String_t
server::monitor::NetworkObserver::getName()
{
    return m_name;
}

// Get machine-readable identifier of service.
String_t
server::monitor::NetworkObserver::getId()
{
    return m_identifier;
}

// Handle configuration item.
bool
server::monitor::NetworkObserver::handleConfiguration(const String_t& key, const String_t& value)
{
    if (key == m_identifier + ".HOST") {
        m_address.setName(value);
        return true;
    } else if (key == m_identifier + ".PORT") {
        m_address.setService(value);
        return true;
    } else {
        return false;
    }
}

// Determine service status.
server::monitor::Observer::Status
server::monitor::NetworkObserver::checkStatus()
{
    // ex planetscentral/monitor.cc:updateStatus (sort-of)
    String_t result;
    switch (m_flavor) {
     case Web:
        if (callServer(m_networkStack, m_address, "GET / HTTP/1.0\r\nHost: 127.0.0.1\r\nUser-Agent: c2monitor\r\n\r\n", result, true)) {
            if (result.find("\nServer: c2monitor") != String_t::npos) {
                // Seems like we are talking to ourselves, e.g. when the status widget runs as a replacement to the web server
                // in emergency mode. In any case, this means the actual web server is down.
                return Down;
            } else if (result.size() > 12 && result.compare(0, 6, "HTTP/1", 6) == 0 && result.compare(9, 3, "200", 3) == 0) {
                // Looks like a 200 OK.
                return Running;
            } else {
                return Broken;
            }
        } else {
            return Down;
        }

     case Router:
        if (callServer(m_networkStack, m_address, "LIST", result, true)) {
            return Running;
        } else {
            return Down;
        }

     case Redis:
        if (callServer(m_networkStack, m_address, "PING\r\nQUIT\r\n", result, false)) {
            if (result.size() >= 5 && result.compare(0, 5, "+PONG", 5) == 0) {
                return Running;
            } else {
                return Broken;
            }
        } else {
            return Down;
        }

     case Service:
        if (callServer(m_networkStack, m_address, "PING\n", result, true)) {
            if (result.size() > 5 && result.find("PONG") != result.npos) {
                return Running;
            } else {
                return Broken;
            }
        } else {
            return Down;
        }
    }
    return Broken;
}
