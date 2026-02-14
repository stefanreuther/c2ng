/**
  *  \file util/connectionprovider.cpp
  *  \brief Class util::ConnectionProvider
  */

#include "util/connectionprovider.hpp"
#include "afl/net/http/clientconnection.hpp"
#include "afl/net/securenetworkstack.hpp"
#include "afl/string/messages.hpp"
#include "afl/sys/mutexguard.hpp"

util::ConnectionProvider::ConnectionProvider(afl::net::http::Client& client, afl::net::NetworkStack& stack)
    : m_client(client),
      m_networkStack(stack),
      m_secureNetworkStack(),
      m_wake(0),
      m_mutex(),
      m_stop(false),
      m_thread("ConnectionProvider", *this)
{
    m_thread.start();
}

util::ConnectionProvider::~ConnectionProvider()
{ }

void
util::ConnectionProvider::requestNewConnection()
{
    m_wake.post();
}

// Thread:
void
util::ConnectionProvider::run()
{
    try {
        m_secureNetworkStack.reset(new afl::net::SecureNetworkStack(m_networkStack));
    }
    catch (std::exception& e) {
        // FIXME: log it
    }
    while (1) {
        // Wait for something to happen
        m_wake.wait();

        // Stop requested?
        {
            afl::sys::MutexGuard g(m_mutex);
            if (m_stop) {
                break;
            }
        }

        // Create requested connections
        afl::net::Name name;
        String_t scheme;
        while (m_client.getUnsatisfiedTarget(name, scheme)) {
            if (scheme == "http") {
                tryConnect(m_networkStack, name, scheme);
            } else if (scheme == "https" && m_secureNetworkStack.get() != 0) {
                tryConnect(*m_secureNetworkStack, name, scheme);
            } else {
                // Mismatching scheme, request cannot be fulfilled
                m_client.cancelRequestsByTarget(name, scheme,
                                                afl::net::http::ClientRequest::UnsupportedProtocol,
                                                afl::string::Messages::invalidUrl());
            }
        }
    }
}

void
util::ConnectionProvider::stop()
{
    {
        afl::sys::MutexGuard g(m_mutex);
        m_stop = true;
    }
    m_wake.post();
}

void
util::ConnectionProvider::tryConnect(afl::net::NetworkStack& stack, const afl::net::Name& name, const String_t& scheme)
{
    const uint32_t CONNECTION_TIMEOUT = 30000;
    try {
        // Try connecting...
        afl::base::Ref<afl::net::Socket> socket = stack.connect(name, CONNECTION_TIMEOUT);
        m_client.addNewConnection(new afl::net::http::ClientConnection(name, scheme, socket));
    }
    catch (std::exception& e) {
        // Regular failure case
        m_client.cancelRequestsByTarget(name, scheme,
                                        afl::net::http::ClientRequest::ConnectionFailed,
                                        e.what());
    }
    catch (...) {
        // Irregular failure case; avoid that exceptions kill the thread
        m_client.cancelRequestsByTarget(name, scheme,
                                        afl::net::http::ClientRequest::ConnectionFailed,
                                        afl::string::Messages::unknownError());
    }
}
