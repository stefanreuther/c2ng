/**
  *  \file game/test/webserver.hpp
  *  \brief Class game::test::WebServer
  */
#ifndef C2NG_GAME_TEST_WEBSERVER_HPP
#define C2NG_GAME_TEST_WEBSERVER_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/net/http/client.hpp"
#include "afl/net/http/manager.hpp"
#include "afl/net/http/page.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/thread.hpp"

namespace game { namespace test {

    /** Webserver mock for testing API implementations.

        Provides one or more internal web servers and a HTTP client/manager to talk to them,
        packaged in one nice class.

        To use,
        - create, passing a NetworkStack;
        - call addNewPage() repeatedly to add behaviour;
        - use client() or manager(). */
    class WebServer {
     public:
        class Instance;
        class ConnectionProvider;

        /** Constructor.
            @param stack NetworkStack. Normally, pass InternalNetworkStack that allows listening on all host names. */
        explicit WebServer(afl::net::NetworkStack& stack);

        /** Destructor.
            Stops the internal servers. */
        ~WebServer();

        /** Add a new page.

            @param host    Host name.
                           If this name has not been previously used, starts a server to listen on that address.
                           Should include a port number (:80 or :443).
            @param path    Path; see afl::net::http::PageDispatcher::addNewPage()
            @param page    Newly-allocated Page object; WebServer will take ownership */
        void addNewPage(const char* host, const char* path, afl::net::http::Page* page);

        /** Reset all servers.
            Reverts all previous addNewPage() so you can add new pages. */
        void reset();

        /** Get HTTP client.
            This is a convenience method to access a pre-configured client.
            The client is ready to be used and can create connections to all host names.
            @return client */
        afl::net::http::Client& client()
            { return m_client; }

        /** Get HTTP manager.
            @return manager */
        afl::net::http::Manager& manager()
            { return m_manager; }

     private:
        afl::net::NetworkStack& m_stack;
        afl::net::http::Client m_client;
        afl::net::http::Manager m_manager;
        afl::sys::Thread m_clientThread;
        afl::container::PtrVector<Instance> m_instances;
    };

} }

#endif
