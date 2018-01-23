/**
  *  \file u/t_server_common_sessionprotocolhandler.cpp
  *  \brief Test for server::common::SessionProtocolHandler
  */

#include "server/common/sessionprotocolhandler.hpp"

#include "t_server_common.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/net/resp/protocolhandler.hpp"

namespace {
    class Tester : public afl::net::CommandHandler {
     public:
        Tester(int& n, String_t& s)
            : m_n(n), m_s(s)
            { }
        Value_t* call(const Segment_t& s)
            {
                m_n = static_cast<int>(s.size());
                m_s += 'x';
                return new afl::data::StringValue(m_s);
            }
        void callVoid(const Segment_t& s)
            { delete call(s); }

        Tester* clone() const
            { return new Tester(*this); }
     private:
        int& m_n;
        String_t m_s;
    };
}

/** Simple test. */
void
TestServerCommonSessionProtocolHandler::testIt()
{
    // Test setup
    int root = 3;
    server::common::SessionProtocolHandler<int, String_t, afl::net::resp::ProtocolHandler, Tester> testee(root);

    // Verify. SessionProtocolHandler is a ProtocolHandler, so send them protocol...
    testee.handleData(afl::string::toBytes("*2\r\n+ok\r\n+ok\r\n"));

    // ...and receive protocol
    String_t result;
    while (1) {
        afl::net::ProtocolHandler::Operation op;
        testee.getOperation(op);
        result.append(afl::string::fromBytes(op.m_dataToSend));
        if (op.m_dataToSend.empty()) {
            break;
        }
    }

    // Must have returned one 'x'.
    TS_ASSERT_EQUALS(result, "$1\r\nx\r\n");

    // Must have set root to 2 because we sent a 2-element array.
    TS_ASSERT_EQUALS(root, 2);
}

