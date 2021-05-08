/**
  *  \file u/t_server_test_consolecommandhandlermock.cpp
  *  \brief Test for server::test::ConsoleCommandHandlerMock
  */

#include "server/test/consolecommandhandlermock.hpp"

#include "t_server_test.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "interpreter/error.hpp"
#include "server/console/environment.hpp"
#include "server/console/nullterminal.hpp"
#include "server/console/parser.hpp"

void
TestServerTestConsoleCommandHandlerMock::testIt()
{
    // Testee
    server::test::ConsoleCommandHandlerMock testee("testIt");

    // Environment
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    server::console::Parser p(env, term, fs, testee);

    afl::data::Segment seg;
    seg.pushBackInteger(7);


    // First test: Success
    {
        interpreter::Arguments args(seg, 0, 0);

        testee.expectCall("foo");
        testee.provideReturnValue(server::test::ConsoleCommandHandlerMock::Success, new afl::data::IntegerValue(12));

        std::auto_ptr<afl::data::Value> result;
        TS_ASSERT_EQUALS(testee.call("foo", args, p, result), true);

        afl::data::IntegerValue* iv = dynamic_cast<afl::data::IntegerValue*>(result.get());
        TS_ASSERT(iv != 0);
        TS_ASSERT_EQUALS(iv->getValue(), 12);
    }

    // Second test: Unrecognized
    {
        interpreter::Arguments args(seg, 0, 0);

        testee.expectCall("foo");
        testee.provideReturnValue(server::test::ConsoleCommandHandlerMock::Unrecognized, 0);

        std::auto_ptr<afl::data::Value> result;
        TS_ASSERT_EQUALS(testee.call("foo", args, p, result), false);
    }

    // Third test: Failure
    {
        interpreter::Arguments args(seg, 0, 1);

        testee.expectCall("bar|7");
        testee.provideReturnValue(server::test::ConsoleCommandHandlerMock::Failure, 0);

        std::auto_ptr<afl::data::Value> result;
        TS_ASSERT_THROWS(testee.call("bar", args, p, result), interpreter::Error);
    }

    // Fourth test: Mismatch (assertion failure)
    {
        interpreter::Arguments args(seg, 0, 0);

        testee.expectCall("mismatch");
        testee.provideReturnValue(server::test::ConsoleCommandHandlerMock::Failure, 0);

        std::auto_ptr<afl::data::Value> result;
        TS_ASSERT_THROWS(testee.call("bar", args, p, result), afl::except::AssertionFailedException);
    }
}

