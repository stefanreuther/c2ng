/**
  *  \file test/server/common/sessionprotocolhandlertest.cpp
  *  \brief Test for server::common::SessionProtocolHandler
  */

#include "server/common/sessionprotocolhandler.hpp"

#include "afl/data/stringvalue.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/test/testrunner.hpp"

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
AFL_TEST("server.common.SessionProtocolHandler", a)
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
    a.checkEqual("01. result", result, "$1\r\nx\r\n");

    // Must have set root to 2 because we sent a 2-element array.
    a.checkEqual("11. root", root, 2);
}
