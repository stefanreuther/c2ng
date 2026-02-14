/**
  *  \file test/util/connectionprovidertest.cpp
  *  \brief Test for util::ConnectionProvider
  */

#include "util/connectionprovider.hpp"

#include "afl/net/http/dispatcher.hpp"
#include "afl/net/http/manager.hpp"
#include "afl/net/http/protocolhandler.hpp"
#include "afl/net/http/request.hpp"
#include "afl/net/http/response.hpp"
#include "afl/net/http/simpledownloadlistener.hpp"
#include "afl/net/internalnetworkstack.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/server.hpp"
#include "afl/sys/thread.hpp"
#include "afl/test/testrunner.hpp"

using afl::base::Ref;
using afl::net::InternalNetworkStack;
using afl::net::Name;
using afl::net::Server;
using afl::net::Url;
using afl::net::http::Client;
using afl::net::http::Manager;
using afl::net::http::SimpleDownloadListener;
using afl::sys::Thread;

/* Simple smoke test: pass a single HTTP transaction through our ConnectionProvider */
AFL_TEST("util.ConnectionProvider", a)
{
    Ref<InternalNetworkStack> net = InternalNetworkStack::create();
    Name name("host", "444");

    // Server side
    class DispatcherImpl : public afl::net::http::Dispatcher {
     public:
        virtual afl::net::http::Response* createNewResponse(std::auto_ptr<afl::net::http::Request>& /*request*/)
            { return 0; }
    };
    class PHFImpl : public afl::net::ProtocolHandlerFactory {
     public:
        virtual afl::net::ProtocolHandler* create()
            { return new afl::net::http::ProtocolHandler(m_dispatcher); }
     private:
        DispatcherImpl m_dispatcher;
    };
    PHFImpl phf;
    Server server(net->listen(name, 10), phf);
    Thread serverThread("Server", server);

    // Client side including object under test
    Client client;
    client.setNewConnectionProvider(new util::ConnectionProvider(client, *net));
    Thread clientThread("Client", client);

    // Start
    serverThread.start();
    clientThread.start();

    // Execute a request
    SimpleDownloadListener sdl;
    Manager mgr(client);
    Url url;
    a.check("00. url valid", url.parse("http://host:444/file"));
    mgr.getFile(url, sdl);

    SimpleDownloadListener::Status st = sdl.wait();

    // Verify
    a.checkEqual("01. status", st, SimpleDownloadListener::Succeeded);
    a.checkEqual("02. code", sdl.getStatusCode(), 404);
}

