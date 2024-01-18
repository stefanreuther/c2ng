/**
  *  \file test/server/test/consolecommandhandlermocktest.cpp
  *  \brief Test for server::test::ConsoleCommandHandlerMock
  */

#include "server/test/consolecommandhandlermock.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "server/console/environment.hpp"
#include "server/console/nullterminal.hpp"
#include "server/console/parser.hpp"

AFL_TEST("server.test.ConsoleCommandHandlerMock", a)
{
    // Testee
    server::test::ConsoleCommandHandlerMock testee(a);

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
        a.checkEqual("01. call", testee.call("foo", args, p, result), true);

        afl::data::IntegerValue* iv = dynamic_cast<afl::data::IntegerValue*>(result.get());
        a.checkNonNull("11. result", iv);
        a.checkEqual("12. getValue", iv->getValue(), 12);
    }

    // Second test: Unrecognized
    {
        interpreter::Arguments args(seg, 0, 0);

        testee.expectCall("foo");
        testee.provideReturnValue(server::test::ConsoleCommandHandlerMock::Unrecognized, 0);

        std::auto_ptr<afl::data::Value> result;
        a.checkEqual("21. call", testee.call("foo", args, p, result), false);
    }

    // Third test: Failure
    {
        interpreter::Arguments args(seg, 0, 1);

        testee.expectCall("bar|7");
        testee.provideReturnValue(server::test::ConsoleCommandHandlerMock::Failure, 0);

        std::auto_ptr<afl::data::Value> result;
        AFL_CHECK_THROWS(a("31. call"), testee.call("bar", args, p, result), interpreter::Error);
    }

    // Fourth test: Mismatch (assertion failure)
    {
        interpreter::Arguments args(seg, 0, 0);

        testee.expectCall("mismatch");
        testee.provideReturnValue(server::test::ConsoleCommandHandlerMock::Failure, 0);

        std::auto_ptr<afl::data::Value> result;
        AFL_CHECK_THROWS(a("41. call"), testee.call("bar", args, p, result), afl::except::AssertionFailedException);
    }
}
