/**
  *  \file game/test/webserver.cpp
  *  \brief Class game::test::WebServer
  */

#include <memory>
#include "game/test/webserver.hpp"
#include "afl/net/http/clientconnection.hpp"
#include "afl/net/http/clientconnectionprovider.hpp"
#include "afl/net/http/clientrequest.hpp"
#include "afl/net/http/pagedispatcher.hpp"
#include "afl/net/http/protocolhandler.hpp"
#include "afl/net/listener.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/server.hpp"
#include "afl/sys/thread.hpp"

using afl::base::Ref;
using afl::net::Listener;
using afl::net::Name;
using afl::net::ProtocolHandlerFactory;
using afl::net::Server;
using afl::net::http::ClientConnection;
using afl::net::http::ClientConnectionProvider;
using afl::net::http::ClientRequest;
using afl::net::http::Page;
using afl::net::http::PageDispatcher;
using afl::net::http::ProtocolHandler;
using afl::sys::Thread;

/*
 *  Instance - server instance for one host name
 */

class game::test::WebServer::Instance : private ProtocolHandlerFactory {
 public:
    Instance(const String_t& host, const Ref<Listener>& listener)
        : m_host(host),
          m_pageDispatcher(),
          m_server(),
          m_thread()
        {
            m_server.reset(new Server(listener, *this));
            m_thread.reset(new Thread("WebServer.Instance", *m_server));
            m_thread->start();
        }

    ~Instance()
        { m_server->stop(); }

    virtual ProtocolHandler* create()
        { return new ProtocolHandler(m_pageDispatcher); }

    void addNewPage(const char* path, Page* page)
        { m_pageDispatcher.addNewPage(path, page); }

    const String_t& getHostName() const
        { return m_host; }

 private:
    String_t m_host;
    PageDispatcher m_pageDispatcher;
    std::auto_ptr<Server> m_server;
    std::auto_ptr<Thread> m_thread;
};


/*
 *  ConnectionProvider - create connections for HTTP client as needed
 */

class game::test::WebServer::ConnectionProvider : public ClientConnectionProvider {
 public:
    ConnectionProvider(WebServer& parent)
        : m_parent(parent)
        { }
    virtual void requestNewConnection()
        {
            Name name;
            String_t scheme;
            while (m_parent.m_client.getUnsatisfiedTarget(name, scheme)) {
                try {
                    m_parent.m_client.addNewConnection(new ClientConnection(name, scheme, m_parent.m_stack.connect(name, 10)));
                }
                catch (std::exception& e) {
                    m_parent.m_client.cancelRequestsByTarget(name, scheme, ClientRequest::ConnectionFailed, e.what());
                }
            }
        }
 private:
    WebServer& m_parent;
};


/*
 *  WebServer
 */

game::test::WebServer::WebServer(afl::net::NetworkStack& stack)
    : m_stack(stack),
      m_client(),
      m_manager(m_client),
      m_clientThread("WebServer.ClientThread", m_client),
      m_instances()
{
    m_client.setNewConnectionProvider(new ConnectionProvider(*this));
    m_clientThread.start();
}

game::test::WebServer::~WebServer()
{
    m_client.stop();
}

void
game::test::WebServer::addNewPage(const char* host, const char* path, afl::net::http::Page* page)
{
    Instance* p = 0;
    for (size_t i = 0; i < m_instances.size(); ++i) {
        if (m_instances[i]->getHostName() == host) {
            p = m_instances[i];
            break;
        }
    }

    if (p == 0) {
        p = m_instances.pushBackNew(new Instance(host, m_stack.listen(Name::parse(host, "80"), 10)));
    }
    p->addNewPage(path, page);
}

void
game::test::WebServer::reset()
{
    m_instances.clear();
}
