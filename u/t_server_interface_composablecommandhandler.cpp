/**
  *  \file u/t_server_interface_composablecommandhandler.cpp
  *  \brief Test for server::interface::ComposableCommandHandler
  */

#include "server/interface/composablecommandhandler.hpp"

#include "t_server_interface.hpp"
#include "server/types.hpp"
#include "afl/data/segment.hpp"

using afl::data::Segment;

/** Interface test. */
void
TestServerInterfaceComposableCommandHandler::testIt()
{
    class Tester : public server::interface::ComposableCommandHandler {
     public:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
            {
                if (upcasedCommand == "X") {
                    result.reset(server::makeIntegerValue(int32_t(args.getNumArgs())));
                    return true;
                } else {
                    return false;
                }
            }
    };
    Tester t;

    Segment empty;
    TS_ASSERT_THROWS(t.call(empty), std::exception);
    TS_ASSERT_THROWS(t.callVoid(empty), std::exception);

    TS_ASSERT_THROWS(t.callVoid(Segment().pushBackString("")), std::exception);
    TS_ASSERT_THROWS(t.callVoid(Segment().pushBackString("Y")), std::exception);

    TS_ASSERT_EQUALS(t.callInt(Segment().pushBackString("X")), 0);
    TS_ASSERT_EQUALS(t.callInt(Segment().pushBackString("x")), 0);
    TS_ASSERT_EQUALS(t.callInt(Segment().pushBackString("x").pushBackString("y").pushBackInteger(9)), 2);
}
