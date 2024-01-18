/**
  *  \file test/server/common/sessionprotocolhandlerfactorytest.cpp
  *  \brief Test for server::common::SessionProtocolHandlerFactory
  */

#include "server/common/sessionprotocolhandlerfactory.hpp"

#include "afl/net/commandhandler.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    class Tester : public afl::net::CommandHandler {
     public:
        Tester(int&, String_t&)
            { }
        Value_t* call(const Segment_t&)
            { return 0; }
        void callVoid(const Segment_t&)
            { }
        Tester* clone() const
            { return new Tester(*this); }
    };
}

/** Simple test. */
AFL_TEST("server.common.SessionProtocolHandlerFactory", a)
{
    int root = 9;
    server::common::SessionProtocolHandlerFactory<int, String_t, afl::net::resp::ProtocolHandler, Tester> testee(root);

    std::auto_ptr<server::common::SessionProtocolHandler<int, String_t, afl::net::resp::ProtocolHandler, Tester> >
        p(testee.create()),
        q(testee.create());

    a.checkNonNull("01. create", p.get());
    a.checkNonNull("02. create", q.get());
    a.checkDifferent("03. unique", p.get(), q.get());
}
