/**
  *  \file util/connectionprovider.hpp
  *  \brief Class util::ConnectionProvider
  */
#ifndef C2NG_UTIL_CONNECTIONPROVIDER_HPP
#define C2NG_UTIL_CONNECTIONPROVIDER_HPP

#include "afl/base/stoppable.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/net/http/client.hpp"
#include "afl/net/http/clientconnectionprovider.hpp"
#include "afl/net/name.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/mutex.hpp"
#include "afl/sys/semaphore.hpp"
#include "afl/sys/thread.hpp"

namespace util {

    /** ClientConnectionProvider implementation for c2ng.
        Provides support for "http" and "https" connections. */
    class ConnectionProvider : public afl::net::http::ClientConnectionProvider,
                               private afl::base::Stoppable,
                               private afl::base::Uncopyable
    {
     public:
        /** Constructor.
            @param client Client to serve
            @param stack  Underlying network stack */
        ConnectionProvider(afl::net::http::Client& client, afl::net::NetworkStack& stack);

        /** Destructor. */
        ~ConnectionProvider();

        // ClientConnectionProvider:
        void requestNewConnection();

     private:
        // Thread:
        virtual void run();
        virtual void stop();
        void tryConnect(afl::net::NetworkStack& stack, const afl::net::Name& name, const String_t& scheme);

        // Integration:
        afl::net::http::Client& m_client;
        afl::net::NetworkStack& m_networkStack;
        std::auto_ptr<afl::net::NetworkStack> m_secureNetworkStack;

        // Work:
        afl::sys::Semaphore m_wake;
        afl::sys::Mutex m_mutex;
        bool m_stop;

        // Thread: must be last
        afl::sys::Thread m_thread;
    };

}

#endif
