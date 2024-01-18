/**
  *  \file test/server/interface/composablecommandhandlertest.cpp
  *  \brief Test for server::interface::ComposableCommandHandler
  */

#include "server/interface/composablecommandhandler.hpp"

#include "afl/data/segment.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

using afl::data::Segment;

AFL_TEST("server.interface.ComposableCommandHandler", a)
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
    AFL_CHECK_THROWS(a("01. empty"), t.call(empty), std::exception);
    AFL_CHECK_THROWS(a("02. empty"), t.callVoid(empty), std::exception);

    AFL_CHECK_THROWS(a("11. null"), t.callVoid(Segment().pushBackString("")), std::exception);
    AFL_CHECK_THROWS(a("12. unknown"), t.callVoid(Segment().pushBackString("Y")), std::exception);

    a.checkEqual("21. known command", t.callInt(Segment().pushBackString("X")), 0);
    a.checkEqual("22. known command", t.callInt(Segment().pushBackString("x")), 0);
    a.checkEqual("23. known command", t.callInt(Segment().pushBackString("x").pushBackString("y").pushBackInteger(9)), 2);
}
