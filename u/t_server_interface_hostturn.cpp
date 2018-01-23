/**
  *  \file u/t_server_interface_hostturn.cpp
  *  \brief Test for server::interface::HostTurn
  */

#include "server/interface/hostturn.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceHostTurn::testInterface()
{
    class Tester : public server::interface::HostTurn {
     public:
        virtual Result submit(const String_t& /*blob*/, afl::base::Optional<int32_t> /*game*/, afl::base::Optional<int32_t> /*slot*/, afl::base::Optional<String_t> /*mail*/, afl::base::Optional<String_t> /*info*/)
            { return Result(); }
        virtual void setTemporary(int32_t /*gameId*/, int32_t /*slot*/, bool /*flag*/)
            { }
    };
    Tester t;
}

/** Test initialisation of Result. */
void
TestServerInterfaceHostTurn::testInit()
{
    server::interface::HostTurn::Result t;
    TS_ASSERT_EQUALS(t.state, 0);
    TS_ASSERT_EQUALS(t.output, "");
    TS_ASSERT_EQUALS(t.gameId, 0);
    TS_ASSERT_EQUALS(t.slot, 0);
    TS_ASSERT_EQUALS(t.previousState, 0);
    TS_ASSERT_EQUALS(t.userId, "");
}
