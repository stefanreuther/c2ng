/**
  *  \file u/t_server_console_parser.cpp
  *  \brief Test for server::console::Parser
  */

#include "server/console/parser.hpp"

#include <stdexcept>
#include "t_server_console.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "server/console/environment.hpp"
#include "server/console/nullterminal.hpp"
#include "server/console/terminal.hpp"
#include "server/test/consolecommandhandlermock.hpp"
#include "server/types.hpp"

using server::test::ConsoleCommandHandlerMock;

/** Test basic evaluation. */
void
TestServerConsoleParser::testEval()
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock("testEval");
    server::console::Parser testee(env, term, fs, mock);

    // Trivial cases
    // - empty
    {
        std::auto_ptr<afl::data::Value> p;
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("", p));
        TS_ASSERT(p.get() == 0);
    }

    // - multiple empty
    {
        std::auto_ptr<afl::data::Value> p;
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("\n\n#foo\n \n", p));
        TS_ASSERT(p.get() == 0);
    }

    // - simple recognized command
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|b|c");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("a b c", p));
        TS_ASSERT(p.get() == 0);
    }

    // - simple recognized command with result
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("q");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(99));
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("  q ", p));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 99);
    }

    // - simple unrecognized command
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|b|c");
        mock.provideReturnValue(mock.Unrecognized, 0);
        TS_ASSERT_THROWS(testee.evaluateString("a b c", p), std::exception);
    }

    // - simple failing command
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|b|c");
        mock.provideReturnValue(mock.Failure, 0);
        TS_ASSERT_THROWS(testee.evaluateString("a b c", p), std::exception);
    }

    // Combination
    // - second command has no result
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|b|c");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(99));
        mock.expectCall("x|y|z");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("a b c\nx y z", p));
        TS_ASSERT(p.get() == 0);
    }

    // - second command has result
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|b|c");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(99));
        mock.expectCall("x|y|z");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(12));
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("a b c\nx y z", p));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 12);
    }
}


/** Test strings/quoting. */
void
TestServerConsoleParser::testString()
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock("testString");
    server::console::Parser testee(env, term, fs, mock);

    // - double quoted
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|a b");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("s \"a b\"", p));
    }

    // - single quoted
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|a b");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("s 'a b'", p));
    }

    // - quotes between word parts
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|a b");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("s a' 'b", p));
    }

    // - brace quoted
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|a b");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("s {a b}", p));
    }

    // - brace quoted, with newlines
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|a\nb\n");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("s {\na\nb\n}", p));
    }

    // - brace quoted, continuing a word (leading newline NOT ignored)
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|x\na\nb\ny");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("s x{\na\nb\n}y", p));
    }

    // - brace quoted with embedded quotes
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|a \"foo\\\"}\"");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("s {a \"foo\\\"}\"}", p));
    }

    // - brace quoted with embedded quotes
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|\"a\" 'b'");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("s {\"a\" 'b'}", p));
    }

    // - byte escape
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|\xc3\xb6");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("s \"\\xc3\\xb6\"", p));
    }
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|\xc3\xb6");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("s \"\\xC3\\xB6\"", p));
    }

    // - unicode escape
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|\xc3\xb6");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("s \"\\""u00f6\"", p));
    }
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|\xc3\xb6");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("s \"\\""u00F6\"", p));
    }

    // - C escape
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|\n");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("s \"\\n\"", p));
    }
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|\r\t'\"");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("s \"\\r\\t\\'\\\"\"", p));
    }

    mock.checkFinish();
}

/** Test pipe behaviour. */
void
TestServerConsoleParser::testPipe()
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock("testPipe");
    server::console::Parser testee(env, term, fs, mock);

    // 2-element pipe
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(7));
        mock.expectCall("b|7");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(12));
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("a | b", p));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 12);
    }

    // 3-element pipe
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(7));
        mock.expectCall("b|7");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(12));
        mock.expectCall("c|12");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(38));
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("a | b|c", p));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 38);
    }

    // Pipe with array result
    {
        afl::data::Segment res;
        res.pushBackString("u");
        res.pushBackString("v");

        std::auto_ptr<afl::data::Value> p;

        mock.expectCall("a");
        mock.provideReturnValue(mock.Success, new afl::data::VectorValue(afl::data::Vector::create(res)));
        mock.expectCall("b|x|u|v");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(12));
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("a | b x", p));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 12);
    }

    mock.checkFinish();
}

/** Test variables. */
void
TestServerConsoleParser::testVar()
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock("testVar");
    server::console::Parser testee(env, term, fs, mock);

    env.setNew("a", server::console::Environment::ValuePtr_t(server::makeIntegerValue(3)));
    env.setNew("q", server::console::Environment::ValuePtr_t(server::makeIntegerValue(7)));
    env.setNew("qq", server::console::Environment::ValuePtr_t(server::makeIntegerValue(9)));

    // Expand variables
    // - normal
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|3|9");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("a ${a} ${qq}", p));
    }

    // - no brace
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|3|7q");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("a $a $qq", p));
    }

    // - quoted
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|3 9");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("a \"${a} ${qq}\"", p));
    }

    // - single-quoted (no expansion)
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|${a} ${qq}");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("a '${a} ${qq}'", p));
    }

    // - braced (no expansion)
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|${a} ${qq}");
        mock.provideReturnValue(mock.Success, 0);
        TS_ASSERT_THROWS_NOTHING(testee.evaluateString("a {${a} ${qq}}", p));
    }
    mock.checkFinish();
}

/** Test parser errors. These should not hit the CommandHandler. */
void
TestServerConsoleParser::testErrors()
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock("testErrors");
    server::console::Parser testee(env, term, fs, mock);
    std::auto_ptr<afl::data::Value> p;

    // No verb in pipe
    TS_ASSERT_THROWS(testee.evaluateString("| x", p), std::exception);

    // Bad escapes
    TS_ASSERT_THROWS(testee.evaluateString("a \"\\""xX\"", p), std::exception);
    TS_ASSERT_THROWS(testee.evaluateString("a \"\\""x1X\"", p), std::exception);
    TS_ASSERT_THROWS(testee.evaluateString("a \"\\""uX\"", p), std::exception);
    TS_ASSERT_THROWS(testee.evaluateString("a \"\\""u1X\"", p), std::exception);
    TS_ASSERT_THROWS(testee.evaluateString("a \"\\""u11X\"", p), std::exception);
    TS_ASSERT_THROWS(testee.evaluateString("a \"\\""u111X\"", p), std::exception);

    // Missing file name
    TS_ASSERT_THROWS(testee.evaluateString("a <", p), std::exception);

    // Bad variable reference
    TS_ASSERT_THROWS(testee.evaluateString("a $$", p), std::exception);
    TS_ASSERT_THROWS(testee.evaluateString("a $ ", p), std::exception);
    TS_ASSERT_THROWS(testee.evaluateString("a $", p), std::exception);
    TS_ASSERT_THROWS(testee.evaluateString("a ${", p), std::exception);

    // Unpaired quotes
    TS_ASSERT_THROWS(testee.evaluateString("a 'x", p), std::exception);
    TS_ASSERT_THROWS(testee.evaluateString("a \"x", p), std::exception);
    TS_ASSERT_THROWS(testee.evaluateString("a {x", p), std::exception);
}

/** Test evaluateStringToBool(). */
void
TestServerConsoleParser::testEvalBool()
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock("testEvalBool");
    server::console::Parser testee(env, term, fs, mock);

    // Null
    mock.expectCall("g|1");
    mock.provideReturnValue(mock.Success, 0);
    TS_ASSERT_EQUALS(testee.evaluateStringToBool("g 1"), false);

    // Zero
    mock.expectCall("g|1");
    mock.provideReturnValue(mock.Success, server::makeIntegerValue(0));
    TS_ASSERT_EQUALS(testee.evaluateStringToBool("g 1"), false);

    // Nonzero
    mock.expectCall("g|1");
    mock.provideReturnValue(mock.Success, server::makeIntegerValue(3));
    TS_ASSERT_EQUALS(testee.evaluateStringToBool("g 1"), true);

    // Empty string
    mock.expectCall("g|1");
    mock.provideReturnValue(mock.Success, server::makeStringValue(""));
    TS_ASSERT_EQUALS(testee.evaluateStringToBool("g 1"), false);

    // Nonempty string
    mock.expectCall("g|1");
    mock.provideReturnValue(mock.Success, server::makeStringValue("q"));
    TS_ASSERT_EQUALS(testee.evaluateStringToBool("g 1"), true);

    mock.checkFinish();
}

/** Test piping with empty result. */
void
TestServerConsoleParser::testEmptyPipe()
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock("testEmptyPipe");
    server::console::Parser testee(env, term, fs, mock);

    // Pipe with empty array result
    mock.expectCall("a");
    mock.provideReturnValue(mock.Success, new afl::data::VectorValue(afl::data::Vector::create()));

    mock.expectCall("b|x");
    mock.provideReturnValue(mock.Success, server::makeIntegerValue(42));
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_THROWS_NOTHING(testee.evaluateString("a | b x", p));
    TS_ASSERT_EQUALS(server::toInteger(p.get()), 42);

    mock.checkFinish();
}

/** Test piping with different result types. */
void
TestServerConsoleParser::testTypedPipe()
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock("testTypedPipe");
    server::console::Parser testee(env, term, fs, mock);

    // Pipe with empty array result
    mock.expectCall("bo");
    mock.provideReturnValue(mock.Success, new afl::data::BooleanValue(true));

    mock.expectCall("in|true");
    mock.provideReturnValue(mock.Success, server::makeIntegerValue(42));

    mock.expectCall("fl|42");
    mock.provideReturnValue(mock.Success, new afl::data::FloatValue(7.5));

    mock.expectCall("st|7.5");
    mock.provideReturnValue(mock.Success, server::makeStringValue("s"));

    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_THROWS_NOTHING(testee.evaluateString("bo|in|fl|st", p));
    TS_ASSERT_EQUALS(server::toString(p.get()), "s");

    mock.checkFinish();
}

