/**
  *  \file test/server/console/fundamentalcommandhandlertest.cpp
  *  \brief Test for server::console::FundamentalCommandHandler
  */

#include "server/console/fundamentalcommandhandler.hpp"

#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/internaltextwriter.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/test/testrunner.hpp"
#include "server/console/environment.hpp"
#include "server/console/nullterminal.hpp"
#include "server/console/parser.hpp"
#include "server/console/pipeterminal.hpp"
#include "server/test/consolecommandhandlermock.hpp"
#include "server/types.hpp"
#include <stdexcept>

using server::console::FundamentalCommandHandler;
using server::test::ConsoleCommandHandlerMock;

namespace {

    class TestHarness {
     public:
        TestHarness(afl::test::Assert a)
            : m_environment(),
              m_terminal(),
              m_fileSystem(),
              m_mock(a),
              m_parser(m_environment, m_terminal, m_fileSystem, m_mock)
            { }

        ConsoleCommandHandlerMock& mock()
            { return m_mock; }

        server::console::Environment& env()
            { return m_environment; }

        server::console::Parser& parser()
            { return m_parser; }

     private:
        server::console::Environment m_environment;
        server::console::NullTerminal m_terminal;
        afl::io::NullFileSystem m_fileSystem;
        ConsoleCommandHandlerMock m_mock;
        server::console::Parser m_parser;
    };
}

/** Test forach, default case.
    Verifies that regular foreach operation succeeds. */
AFL_TEST("server.console.FundamentalCommandHandler:foreach", a)
{
    // Environment
    TestHarness h(a);

    // Command
    afl::data::Segment seg;
    seg.pushBackString("i");
    seg.pushBackString("echo $i");
    seg.pushBackString("a");
    seg.pushBackString("b");
    seg.pushBackString("c");

    // Expected result
    h.mock().expectCall("echo|a"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, 0);
    h.mock().expectCall("echo|b"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, 0);
    h.mock().expectCall("echo|c"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, 0);

    // Call
    FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    a.check("01. call", testee.call("foreach", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    a.checkNull("11. result", result.get());
    a.checkNull("12. env i", h.env().get("i"));
    h.mock().checkFinish();
}

/** Test foreach, previous value in iteration variable preserved. */
AFL_TEST("server.console.FundamentalCommandHandler:foreach:preserve-previous", a)
{
    // Environment
    TestHarness h(a);
    h.env().setNew("i", std::auto_ptr<afl::data::Value>(server::makeIntegerValue(52)));

    // Command
    afl::data::Segment seg;
    seg.pushBackString("i");
    seg.pushBackString("echo $i");
    seg.pushBackString("x");

    // Expected result
    h.mock().expectCall("echo|x"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, 0);

    // Call
    FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    a.check("01. call", testee.call("foreach", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    a.checkNull("11. result", result.get());
    a.checkEqual("12. env i", server::toInteger(h.env().get("i")), 52);
    h.mock().checkFinish();
}

/** Test foreach, previous value in iteration variable preserved even in case of error. */
AFL_TEST("server.console.FundamentalCommandHandler:foreach:command-fails", a)
{
    // Environment
    TestHarness h(a);
    h.env().setNew("i", std::auto_ptr<afl::data::Value>(server::makeIntegerValue(32168)));

    // Command
    afl::data::Segment seg;
    seg.pushBackString("i");
    seg.pushBackString("echo $i");
    seg.pushBackString("x");
    seg.pushBackString("y");

    // Expected result
    h.mock().expectCall("echo|x"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, 0);
    h.mock().expectCall("echo|y"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Failure, 0);

    // Call
    FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    AFL_CHECK_THROWS(a("01. call"), testee.call("foreach", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result), std::exception);

    // Verify
    a.checkEqual("11. env i", server::toInteger(h.env().get("i")), 32168);
    h.mock().checkFinish();
}

/** Test foreach, command is unrecognized. */
AFL_TEST("server.console.FundamentalCommandHandler:foreach:unrecognized-command", a)
{
    // Environment
    TestHarness h(a);
    h.env().setNew("i", std::auto_ptr<afl::data::Value>(server::makeIntegerValue(32168)));

    // Command
    afl::data::Segment seg;
    seg.pushBackString("i");
    seg.pushBackString("echo $i");
    seg.pushBackString("x");
    seg.pushBackString("y");

    // Expected result
    h.mock().expectCall("echo|x"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Unrecognized, 0);

    // Call
    FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    AFL_CHECK_THROWS(a("01. call"), testee.call("foreach", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result), std::exception);

    // Verify
    a.checkEqual("11. env i", server::toInteger(h.env().get("i")), 32168);
    h.mock().checkFinish();
}

/** Test if, standard case. */
AFL_TEST("server.console.FundamentalCommandHandler:if:true", a)
{
    // Environment
    TestHarness h(a);

    // Command
    afl::data::Segment seg;
    seg.pushBackString("condcmd condarg");
    seg.pushBackString("thencmd thenarg");

    // Expected result
    h.mock().expectCall("condcmd|condarg"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(1));
    h.mock().expectCall("thencmd|thenarg"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(7));

    // Call
    FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    a.check("01. call", testee.call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    a.checkNull("11. result", result.get());
    h.mock().checkFinish();
}

/** Test if, standard case, condition false. */
AFL_TEST("server.console.FundamentalCommandHandler:if:false", a)
{
    // Environment
    TestHarness h(a);

    // Command
    afl::data::Segment seg;
    seg.pushBackString("condcmd condarg");
    seg.pushBackString("thencmd thenarg");

    // Expected result
    h.mock().expectCall("condcmd|condarg"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(0));

    // Call
    FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    a.check("01. call", testee.call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    a.checkNull("11. result", result.get());
    h.mock().checkFinish();
}

/** Test if/else, standard case. */
AFL_TEST("server.console.FundamentalCommandHandler:if-else:true", a)
{
    // Environment
    TestHarness h(a);

    // Command
    afl::data::Segment seg;
    seg.pushBackString("condcmd condarg");
    seg.pushBackString("thencmd thenarg");
    seg.pushBackString("else");
    seg.pushBackString("elsecmd elsearg");

    // Expected result
    h.mock().expectCall("condcmd|condarg"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(1));
    h.mock().expectCall("thencmd|thenarg"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(7));

    // Call
    FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    a.check("01. call", testee.call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    a.checkNull("11. result", result.get());
    h.mock().checkFinish();
}

/** Test if/else, standard case, condition false. */
AFL_TEST("server.console.FundamentalCommandHandler:if-else:false", a)
{
    // Environment
    TestHarness h(a);

    // Command
    afl::data::Segment seg;
    seg.pushBackString("condcmd condarg");
    seg.pushBackString("thencmd thenarg");
    seg.pushBackString("else");
    seg.pushBackString("elsecmd elsearg");

    // Expected result
    h.mock().expectCall("condcmd|condarg"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(0));
    h.mock().expectCall("elsecmd|elsearg"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(7));

    // Call
    FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    a.check("01. call", testee.call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    a.checkNull("11. result", result.get());
    h.mock().checkFinish();
}

/** Test if/elsif. */
AFL_TEST("server.console.FundamentalCommandHandler:if-elsif:true", a)
{
    // Environment
    TestHarness h(a);

    // Command
    afl::data::Segment seg;
    seg.pushBackString("condcmd condarg");
    seg.pushBackString("thencmd thenarg");
    seg.pushBackString("elsif");
    seg.pushBackString("cond2");
    seg.pushBackString("2nd cmd");
    seg.pushBackString("elsif");
    seg.pushBackString("cond3");
    seg.pushBackString("3rd cmd");

    // Expected result
    h.mock().expectCall("condcmd|condarg"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(0));
    h.mock().expectCall("cond2"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(1));
    h.mock().expectCall("2nd|cmd"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, 0);

    // Call
    FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    a.check("01. call", testee.call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    a.checkNull("11. result", result.get());
    h.mock().checkFinish();
}

/** Test if/elsif, conditions false. */
AFL_TEST("server.console.FundamentalCommandHandler:if-elsif:false", a)
{
    // Environment
    TestHarness h(a);

    // Command
    afl::data::Segment seg;
    seg.pushBackString("condcmd condarg");
    seg.pushBackString("thencmd thenarg");
    seg.pushBackString("elsif");
    seg.pushBackString("cond2");
    seg.pushBackString("2nd cmd");
    seg.pushBackString("elsif");
    seg.pushBackString("cond3");
    seg.pushBackString("3rd cmd");

    // Expected result
    h.mock().expectCall("condcmd|condarg"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(0));
    h.mock().expectCall("cond2"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(0));
    h.mock().expectCall("cond3"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(0));

    // Call
    FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    a.check("01. call", testee.call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    a.checkNull("11. result", result.get());
    h.mock().checkFinish();
}

/** Test if, multiline. */
AFL_TEST("server.console.FundamentalCommandHandler:if:multiline", a)
{
    // Environment
    TestHarness h(a);

    // Command
    afl::data::Segment seg;
    seg.pushBackString("c1\nc2");
    seg.pushBackString("t1\nt2");

    // Expected result
    h.mock().expectCall("c1"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(0));
    h.mock().expectCall("c2"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(1));
    h.mock().expectCall("t1"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(2));
    h.mock().expectCall("t2"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(3));

    // Call
    FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    a.check("01. call", testee.call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    a.checkNull("11. result", result.get());
    h.mock().checkFinish();
}

/** Test setenv. */
AFL_TEST("server.console.FundamentalCommandHandler:setenv", a)
{
    // Environment
    TestHarness h(a);

    // Command
    afl::data::Segment seg;
    seg.pushBackString("vn");
    seg.pushBackString("vv");

    // Call
    FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    a.check("01. call", testee.call("setenv", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    a.checkNull("11. result", result.get());
    a.checkEqual("12. env vn", server::toString(h.env().get("vn")), "vv");
}

/** Test env. */
AFL_TEST("server.console.FundamentalCommandHandler:env", a)
{
    // Environment
    TestHarness h(a);
    h.env().setNew("i", std::auto_ptr<afl::data::Value>(server::makeIntegerValue(52)));
    h.env().setNew("s", std::auto_ptr<afl::data::Value>(server::makeStringValue("q")));

    // Call
    FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    afl::data::Segment empty;
    a.check("01. call", testee.call("env", interpreter::Arguments(empty, 0, 0), h.parser(), result));

    // Verify
    afl::data::Access aa(result.get());
    a.checkNonNull("11. result", result.get());
    a.checkEqual("12. getArraySize", aa.getArraySize(), 4U);
    a.checkEqual("13. result i", aa("i").toInteger(), 52);
    a.checkEqual("14. result s", aa("s").toString(), "q");
}

/** Test echo. */
AFL_TEST("server.console.FundamentalCommandHandler:echo", a)
{
    // Environment (need a terminal here to catch the output)
    server::console::Environment env;
    afl::io::InternalTextWriter out;
    server::console::PipeTerminal terminal(out, out);
    afl::io::NullFileSystem fileSystem;
    ConsoleCommandHandlerMock mock(a);
    server::console::Parser parser(env, terminal, fileSystem, mock);

    // Verify calls
    // - echo (no args)
    {
        std::auto_ptr<afl::data::Value> result;
        afl::data::Segment empty;
        a.check("01. no-args", FundamentalCommandHandler(env).call("echo", interpreter::Arguments(empty, 0, 0), parser, result));
        a.checkEqual("02. result", afl::string::fromMemory(out.getContent()), "\n");
        out.clear();
    }

    // - echo (one arg)
    {
        afl::data::Segment seg;
        seg.pushBackString("xyz");
        std::auto_ptr<afl::data::Value> result;
        a.check("11. one-arg", FundamentalCommandHandler(env).call("echo", interpreter::Arguments(seg, 0, seg.size()), parser, result));
        a.checkEqual("12. result", afl::string::fromMemory(out.getContent()), "xyz\n");
        out.clear();
    }

    // - echo (three args)
    {
        afl::data::Segment seg;
        seg.pushBackString("xyz");
        seg.pushBackInteger(-8);
        seg.pushBackString("q");
        std::auto_ptr<afl::data::Value> result;
        a.check("21. many args", FundamentalCommandHandler(env).call("echo", interpreter::Arguments(seg, 0, seg.size()), parser, result));
        a.checkEqual("22. result", afl::string::fromMemory(out.getContent()), "xyz -8 q\n");
        out.clear();
    }
}

/*
 *  Test various errors.
 */

// Unrecognized
AFL_TEST("server.console.FundamentalCommandHandler:unrecognized-command", a)
{
    TestHarness h(a);
    std::auto_ptr<afl::data::Value> result;
    afl::data::Segment seg;
    a.checkEqual("", FundamentalCommandHandler(h.env()).call("set", interpreter::Arguments(seg, 0, 0), h.parser(), result), false);
}

// Parameter count
AFL_TEST("server.console.FundamentalCommandHandler:bad-parameters", a)
{
    TestHarness h(a);
    std::auto_ptr<afl::data::Value> result;
    afl::data::Segment seg;
    // - env does not take args
    AFL_CHECK_THROWS(a("01. env"), FundamentalCommandHandler(h.env()).call("env", interpreter::Arguments(seg, 0, 1), h.parser(), result), std::exception);

    // - setenv needs 2 args
    AFL_CHECK_THROWS(a("11. setenv"), FundamentalCommandHandler(h.env()).call("setenv", interpreter::Arguments(seg, 0, 1), h.parser(), result), std::exception);
    AFL_CHECK_THROWS(a("12. setenv"), FundamentalCommandHandler(h.env()).call("setenv", interpreter::Arguments(seg, 0, 3), h.parser(), result), std::exception);

    // - if
    AFL_CHECK_THROWS(a("21. if"), FundamentalCommandHandler(h.env()).call("if", interpreter::Arguments(seg, 0, 0), h.parser(), result), std::exception);
    AFL_CHECK_THROWS(a("22. if"), FundamentalCommandHandler(h.env()).call("if", interpreter::Arguments(seg, 0, 1), h.parser(), result), std::exception);
    AFL_CHECK_THROWS(a("23. if"), FundamentalCommandHandler(h.env()).call("if", interpreter::Arguments(seg, 0, 3), h.parser(), result), std::exception);

    // - foreach
    AFL_CHECK_THROWS(a("31. foreach"), FundamentalCommandHandler(h.env()).call("foreach", interpreter::Arguments(seg, 0, 1), h.parser(), result), std::exception);
}

// Bad keywords in if
AFL_TEST("server.console.FundamentalCommandHandler:if:bad-keyword", a)
{
    TestHarness h(a);
    std::auto_ptr<afl::data::Value> result;
    afl::data::Segment seg;
    seg.pushBackString("aa");
    seg.pushBackString("bb");
    seg.pushBackString("cc");    // should be else
    seg.pushBackString("dd");
    h.mock().expectCall("aa"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(0));
    AFL_CHECK_THROWS(a, FundamentalCommandHandler(h.env()).call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result), std::exception);
}
