/**
  *  \file u/t_server_common_sessionprotocolhandlerfactory.cpp
  *  \brief Test for server::common::SessionProtocolHandlerFactory
  */

#include "server/common/sessionprotocolhandlerfactory.hpp"

#include "t_server_common.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/net/resp/protocolhandler.hpp"

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
void
TestServerCommonSessionProtocolHandlerFactory::testIt()
{
    int root = 9;
    server::common::SessionProtocolHandlerFactory<int, String_t, afl::net::resp::ProtocolHandler, Tester> testee(root);

    std::auto_ptr<server::common::SessionProtocolHandler<int, String_t, afl::net::resp::ProtocolHandler, Tester> >
        p(testee.create()),
        q(testee.create());

    TS_ASSERT(p.get() != 0);
    TS_ASSERT(q.get() != 0);
    TS_ASSERT(p.get() != q.get());
}
