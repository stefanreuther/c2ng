/**
  *  \file test/server/console/clientpooltest.cpp
  *  \brief Test for server::console::ClientPool
  */

#include "server/console/clientpool.hpp"

#include "afl/except/assertionfailedexception.hpp"
#include "afl/net/internalnetworkstack.hpp"
#include "afl/net/name.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/sys/thread.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

using afl::net::Name;

namespace {
    class ProtocolHandlerFactory : public afl::net::ProtocolHandlerFactory,
                                   public afl::net::CommandHandler
    {
     public:
        ProtocolHandlerFactory()
            : m_count()
            { }
        virtual afl::net::ProtocolHandler* create()
            { m_count = 0; return new afl::net::resp::ProtocolHandler(*this); }
        virtual Value_t* call(const Segment_t& /*command*/)
            { return server::makeIntegerValue(++m_count); }
        virtual void callVoid(const Segment_t& command)
            { delete call(command); }

     private:
        int m_count;
    };
}

/** Test a simple sequence. */
AFL_TEST("server.console.ClientPool", a)
{
    // Network stack and a server
    afl::base::Ref<afl::net::InternalNetworkStack> net = afl::net::InternalNetworkStack::create();
    ProtocolHandlerFactory fac;
    afl::net::Server server(net->listen(Name("host", 77), 10), fac);
    afl::sys::Thread serverThread("user.server", server);
    serverThread.start();

    // Testee
    server::console::ClientPool testee(*net);

    // Add clients
    server::console::ClientPool::Index_t idx1 = testee.create("foo", 100);
    server::console::ClientPool::Index_t idx2 = testee.create("bar", 200);
    a.checkDifferent("01. different indexes", idx1, idx2);

    // Configure
    a.check("11. config foo host", testee.handleConfiguration("Foo.Host", "host"));
    a.check("12. config foo port", testee.handleConfiguration("Foo.Port", "77"));
    a.check("13. config bar port", testee.handleConfiguration("Bar.Port", "99"));
    a.check("19. config baz port", !testee.handleConfiguration("Baz.Port", "99"));

    // Access a client
    afl::data::Segment empty;
    a.checkEqual("21. call",       testee.get(idx1).callInt(empty), 1);
    a.checkEqual("22. call again", testee.get(idx1).callInt(empty), 2);

    // Reset and call again
    testee.reset(idx1);
    a.checkEqual("31. call",       testee.get(idx1).callInt(empty), 1);
    a.checkEqual("32. call again", testee.get(idx1).callInt(empty), 2);

    // Find
    a.checkEqual("41. find", testee.find("foo"), idx1);
    a.checkEqual("42. find", testee.find("bar"), idx2);
    AFL_CHECK_THROWS(a("43. find"), testee.find("baz"), afl::except::AssertionFailedException);

    // Get error case
    AFL_CHECK_THROWS(a("51. get error"), testee.get(idx2+1), afl::except::AssertionFailedException);

    // Shut down
    server.stop();
    serverThread.join();
}
