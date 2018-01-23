/**
  *  \file u/t_server_console_fundamentalcommandhandler.cpp
  *  \brief Test for server::console::FundamentalCommandHandler
  */

#include "server/console/fundamentalcommandhandler.hpp"

#include <stdexcept>
#include "t_server_console.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "server/console/environment.hpp"
#include "server/console/nullterminal.hpp"
#include "server/console/parser.hpp"
#include "server/console/pipeterminal.hpp"
#include "server/types.hpp"
#include "u/helper/consolecommandhandlermock.hpp"
#include "afl/io/internaltextwriter.hpp"

namespace {
    class TestHarness {
     public:
        TestHarness()
            : m_environment(),
              m_terminal(),
              m_fileSystem(),
              m_mock("TestServerConsoleFundamentalCommandHandler"),
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
void
TestServerConsoleFundamentalCommandHandler::testForEach()
{
    // Environment
    TestHarness h;

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
    server::console::FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    TS_ASSERT(testee.call("foreach", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    TS_ASSERT(result.get() == 0);
    TS_ASSERT(h.env().get("i") == 0);
    h.mock().checkFinish();
}

/** Test foreach, previous value in iteration variable preserved. */
void
TestServerConsoleFundamentalCommandHandler::testForEachPreserve()
{
    // Environment
    TestHarness h;
    h.env().setNew("i", std::auto_ptr<afl::data::Value>(server::makeIntegerValue(52)));

    // Command
    afl::data::Segment seg;
    seg.pushBackString("i");
    seg.pushBackString("echo $i");
    seg.pushBackString("x");

    // Expected result
    h.mock().expectCall("echo|x"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, 0);

    // Call
    server::console::FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    TS_ASSERT(testee.call("foreach", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    TS_ASSERT(result.get() == 0);
    TS_ASSERT_EQUALS(server::toInteger(h.env().get("i")), 52);
    h.mock().checkFinish();
}

/** Test foreach, previous value in iteration variable preserved even in case of error. */
void
TestServerConsoleFundamentalCommandHandler::testForEachError()
{
    // Environment
    TestHarness h;
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
    server::console::FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    TS_ASSERT_THROWS(testee.call("foreach", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result), std::exception);

    // Verify
    TS_ASSERT_EQUALS(server::toInteger(h.env().get("i")), 32168);
    h.mock().checkFinish();
}

/** Test foreach, command is unrecognized. */
void
TestServerConsoleFundamentalCommandHandler::testForEachUnrecognized()
{
    // Environment
    TestHarness h;
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
    server::console::FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    TS_ASSERT_THROWS(testee.call("foreach", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result), std::exception);

    // Verify
    TS_ASSERT_EQUALS(server::toInteger(h.env().get("i")), 32168);
    h.mock().checkFinish();
}

/** Test if, standard case. */
void
TestServerConsoleFundamentalCommandHandler::testIf()
{
    // Environment
    TestHarness h;

    // Command
    afl::data::Segment seg;
    seg.pushBackString("condcmd condarg");
    seg.pushBackString("thencmd thenarg");

    // Expected result
    h.mock().expectCall("condcmd|condarg"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(1));
    h.mock().expectCall("thencmd|thenarg"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(7));

    // Call
    server::console::FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    TS_ASSERT(testee.call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    TS_ASSERT(result.get() == 0);
    h.mock().checkFinish();
}

/** Test if, standard case, condition false. */
void
TestServerConsoleFundamentalCommandHandler::testIfFalse()
{
    // Environment
    TestHarness h;

    // Command
    afl::data::Segment seg;
    seg.pushBackString("condcmd condarg");
    seg.pushBackString("thencmd thenarg");

    // Expected result
    h.mock().expectCall("condcmd|condarg"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(0));

    // Call
    server::console::FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    TS_ASSERT(testee.call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    TS_ASSERT(result.get() == 0);
    h.mock().checkFinish();
}

/** Test if/else, standard case. */
void
TestServerConsoleFundamentalCommandHandler::testIfElse()
{
    // Environment
    TestHarness h;

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
    server::console::FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    TS_ASSERT(testee.call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    TS_ASSERT(result.get() == 0);
    h.mock().checkFinish();
}

/** Test if/else, standard case, condition false. */
void
TestServerConsoleFundamentalCommandHandler::testIfElseFalse()
{
    // Environment
    TestHarness h;

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
    server::console::FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    TS_ASSERT(testee.call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    TS_ASSERT(result.get() == 0);
    h.mock().checkFinish();
}

/** Test if/elsif. */
void
TestServerConsoleFundamentalCommandHandler::testIfElsif()
{
    // Environment
    TestHarness h;

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
    server::console::FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    TS_ASSERT(testee.call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    TS_ASSERT(result.get() == 0);
    h.mock().checkFinish();
}

/** Test if/elsif, conditions false. */
void
TestServerConsoleFundamentalCommandHandler::testIfElsifFalse()
{
    // Environment
    TestHarness h;

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
    server::console::FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    TS_ASSERT(testee.call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    TS_ASSERT(result.get() == 0);
    h.mock().checkFinish();
}

/** Test if, multiline. */
void
TestServerConsoleFundamentalCommandHandler::testIfMultiline()
{
    // Environment
    TestHarness h;

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
    server::console::FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    TS_ASSERT(testee.call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    TS_ASSERT(result.get() == 0);
    h.mock().checkFinish();
}

/** Test setenv. */
void
TestServerConsoleFundamentalCommandHandler::testSetenv()
{
    // Environment
    TestHarness h;

    // Command
    afl::data::Segment seg;
    seg.pushBackString("vn");
    seg.pushBackString("vv");

    // Call
    server::console::FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    TS_ASSERT(testee.call("setenv", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result));

    // Verify
    TS_ASSERT(result.get() == 0);
    TS_ASSERT_EQUALS(server::toString(h.env().get("vn")), "vv");
}

/** Test env. */
void
TestServerConsoleFundamentalCommandHandler::testEnv()
{
    // Environment
    TestHarness h;
    h.env().setNew("i", std::auto_ptr<afl::data::Value>(server::makeIntegerValue(52)));
    h.env().setNew("s", std::auto_ptr<afl::data::Value>(server::makeStringValue("q")));

    // Call
    server::console::FundamentalCommandHandler testee(h.env());
    std::auto_ptr<afl::data::Value> result;
    afl::data::Segment empty;
    TS_ASSERT(testee.call("env", interpreter::Arguments(empty, 0, 0), h.parser(), result));

    // Verify
    afl::data::Access a(result.get());
    TS_ASSERT(result.get() != 0);
    TS_ASSERT_EQUALS(a.getArraySize(), 4U);
    TS_ASSERT_EQUALS(a("i").toInteger(), 52);
    TS_ASSERT_EQUALS(a("s").toString(), "q");
}

/** Test echo. */
void
TestServerConsoleFundamentalCommandHandler::testEcho()
{
    // Environment (need a terminal here to catch the output)
    server::console::Environment env;
    afl::io::InternalTextWriter out;
    server::console::PipeTerminal terminal(out, out);
    afl::io::NullFileSystem fileSystem;
    ConsoleCommandHandlerMock mock("testEcho");
    server::console::Parser parser(env, terminal, fileSystem, mock);

    // Verify calls
    // - echo (no args)
    {
        std::auto_ptr<afl::data::Value> result;
        afl::data::Segment empty;
        TS_ASSERT(server::console::FundamentalCommandHandler(env).call("echo", interpreter::Arguments(empty, 0, 0), parser, result));
        TS_ASSERT_EQUALS(afl::string::fromMemory(out.getContent()), "\n");
        out.clear();
    }

    // - echo (one arg)
    {
        afl::data::Segment seg;
        seg.pushBackString("xyz");
        std::auto_ptr<afl::data::Value> result;
        TS_ASSERT(server::console::FundamentalCommandHandler(env).call("echo", interpreter::Arguments(seg, 0, seg.size()), parser, result));
        TS_ASSERT_EQUALS(afl::string::fromMemory(out.getContent()), "xyz\n");
        out.clear();
    }

    // - echo (three args)
    {
        afl::data::Segment seg;
        seg.pushBackString("xyz");
        seg.pushBackInteger(-8);
        seg.pushBackString("q");
        std::auto_ptr<afl::data::Value> result;
        TS_ASSERT(server::console::FundamentalCommandHandler(env).call("echo", interpreter::Arguments(seg, 0, seg.size()), parser, result));
        TS_ASSERT_EQUALS(afl::string::fromMemory(out.getContent()), "xyz -8 q\n");
        out.clear();
    }
}

/** Test various errors. */
void
TestServerConsoleFundamentalCommandHandler::testErrors()
{
    // Environment
    TestHarness h;
    std::auto_ptr<afl::data::Value> result;

    // Unrecognized
    {
        afl::data::Segment seg;
        TS_ASSERT_EQUALS(server::console::FundamentalCommandHandler(h.env()).call("set", interpreter::Arguments(seg, 0, 0), h.parser(), result), false);
    }

    // Parameter count
    {
        afl::data::Segment seg;
        // - env does not take args
        TS_ASSERT_THROWS(server::console::FundamentalCommandHandler(h.env()).call("env", interpreter::Arguments(seg, 0, 1), h.parser(), result), std::exception);

        // - setenv needs 2 args
        TS_ASSERT_THROWS(server::console::FundamentalCommandHandler(h.env()).call("setenv", interpreter::Arguments(seg, 0, 1), h.parser(), result), std::exception);
        TS_ASSERT_THROWS(server::console::FundamentalCommandHandler(h.env()).call("setenv", interpreter::Arguments(seg, 0, 3), h.parser(), result), std::exception);

        // - if
        TS_ASSERT_THROWS(server::console::FundamentalCommandHandler(h.env()).call("if", interpreter::Arguments(seg, 0, 0), h.parser(), result), std::exception);
        TS_ASSERT_THROWS(server::console::FundamentalCommandHandler(h.env()).call("if", interpreter::Arguments(seg, 0, 1), h.parser(), result), std::exception);
        TS_ASSERT_THROWS(server::console::FundamentalCommandHandler(h.env()).call("if", interpreter::Arguments(seg, 0, 3), h.parser(), result), std::exception);

        // - foreach
        TS_ASSERT_THROWS(server::console::FundamentalCommandHandler(h.env()).call("foreach", interpreter::Arguments(seg, 0, 1), h.parser(), result), std::exception);
    }

    // Bad keywords in if
    {
        afl::data::Segment seg;
        seg.pushBackString("aa");
        seg.pushBackString("bb");
        seg.pushBackString("cc");    // should be else
        seg.pushBackString("dd");
        h.mock().expectCall("aa"); h.mock().provideReturnValue(ConsoleCommandHandlerMock::Success, server::makeIntegerValue(0));
        TS_ASSERT_THROWS(server::console::FundamentalCommandHandler(h.env()).call("if", interpreter::Arguments(seg, 0, seg.size()), h.parser(), result), std::exception);
    }
}
