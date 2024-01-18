/**
  *  \file test/server/console/parsertest.cpp
  *  \brief Test for server::console::Parser
  */

#include "server/console/parser.hpp"

#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/test/testrunner.hpp"
#include "server/console/environment.hpp"
#include "server/console/nullterminal.hpp"
#include "server/console/terminal.hpp"
#include "server/test/consolecommandhandlermock.hpp"
#include "server/types.hpp"
#include <stdexcept>

using server::test::ConsoleCommandHandlerMock;

/** Test basic evaluation. */
AFL_TEST("server.console.Parser:evaluateString", a)
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock(a);
    server::console::Parser testee(env, term, fs, mock);

    // Trivial cases
    // - empty
    {
        std::auto_ptr<afl::data::Value> p;
        AFL_CHECK_SUCCEEDS(a("01. empty"), testee.evaluateString("", p));
        a.checkNull("02. result", p.get());
    }

    // - multiple empty
    {
        std::auto_ptr<afl::data::Value> p;
        AFL_CHECK_SUCCEEDS(a("11. multiple empty"), testee.evaluateString("\n\n#foo\n \n", p));
        a.checkNull("12. result", p.get());
    }

    // - simple recognized command
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|b|c");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("21. success"), testee.evaluateString("a b c", p));
        a.checkNull("22. result", p.get());
    }

    // - simple recognized command with result
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("q");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(99));
        AFL_CHECK_SUCCEEDS(a("31. success"), testee.evaluateString("  q ", p));
        a.checkEqual("32. result", server::toInteger(p.get()), 99);
    }

    // - simple unrecognized command
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|b|c");
        mock.provideReturnValue(mock.Unrecognized, 0);
        AFL_CHECK_THROWS(a("41. unrecognized"), testee.evaluateString("a b c", p), std::exception);
    }

    // - simple failing command
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|b|c");
        mock.provideReturnValue(mock.Failure, 0);
        AFL_CHECK_THROWS(a("51. failed"), testee.evaluateString("a b c", p), std::exception);
    }

    // Combination
    // - second command has no result
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|b|c");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(99));
        mock.expectCall("x|y|z");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("61. second no result"), testee.evaluateString("a b c\nx y z", p));
        a.checkNull("62. result", p.get());
    }

    // - second command has result
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|b|c");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(99));
        mock.expectCall("x|y|z");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(12));
        AFL_CHECK_SUCCEEDS(a("71. second has result"), testee.evaluateString("a b c\nx y z", p));
        a.checkEqual("72. result", server::toInteger(p.get()), 12);
    }
}

/** Test strings/quoting. */
AFL_TEST("server.console.Parser:evaluateString:strings", a)
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock(a);
    server::console::Parser testee(env, term, fs, mock);

    // - double quoted
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|a b");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("01. double-quote"), testee.evaluateString("s \"a b\"", p));
    }

    // - single quoted
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|a b");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("11. single quote"), testee.evaluateString("s 'a b'", p));
    }

    // - quotes between word parts
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|a b");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("21. middle quote"), testee.evaluateString("s a' 'b", p));
    }

    // - brace quoted
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|a b");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("31. brace quote"), testee.evaluateString("s {a b}", p));
    }

    // - brace quoted, with newlines
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|a\nb\n");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("41. brace with newline"), testee.evaluateString("s {\na\nb\n}", p));
    }

    // - brace quoted, continuing a word (leading newline NOT ignored)
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|x\na\nb\ny");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("51. middle brace"), testee.evaluateString("s x{\na\nb\n}y", p));
    }

    // - brace quoted with embedded quotes
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|a \"foo\\\"}\"");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("61. brace and quote"), testee.evaluateString("s {a \"foo\\\"}\"}", p));
    }

    // - brace quoted with embedded quotes
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|\"a\" 'b'");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("71. brace and quote"), testee.evaluateString("s {\"a\" 'b'}", p));
    }

    // - byte escape
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|\xc3\xb6");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("81. byte-escape"), testee.evaluateString("s \"\\xc3\\xb6\"", p));
    }
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|\xc3\xb6");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("82. byte-escape"), testee.evaluateString("s \"\\xC3\\xB6\"", p));
    }

    // - unicode escape
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|\xc3\xb6");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("91. unicode"), testee.evaluateString("s \"\\""u00f6\"", p));
    }
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|\xc3\xb6");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("92. unicode"), testee.evaluateString("s \"\\""u00F6\"", p));
    }

    // - C escape
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|\n");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("101. C escape"), testee.evaluateString("s \"\\n\"", p));
    }
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("s|\r\t'\"");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("102. C escape"), testee.evaluateString("s \"\\r\\t\\'\\\"\"", p));
    }

    mock.checkFinish();
}

/** Test pipe behaviour. */
AFL_TEST("server.console.Parser:evaluateString:pipe", a)
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock(a);
    server::console::Parser testee(env, term, fs, mock);

    // 2-element pipe
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(7));
        mock.expectCall("b|7");
        mock.provideReturnValue(mock.Success, server::makeIntegerValue(12));
        AFL_CHECK_SUCCEEDS(a("01. 2-elem"), testee.evaluateString("a | b", p));
        a.checkEqual("02. result", server::toInteger(p.get()), 12);
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
        AFL_CHECK_SUCCEEDS(a("11. 3-elem"), testee.evaluateString("a | b|c", p));
        a.checkEqual("12. result", server::toInteger(p.get()), 38);
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
        AFL_CHECK_SUCCEEDS(a("21. intermediate array"), testee.evaluateString("a | b x", p));
        a.checkEqual("22. result", server::toInteger(p.get()), 12);
    }

    mock.checkFinish();
}

/** Test variables. */
AFL_TEST("server.console.Parser:evaluateString:variables", a)
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock(a);
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
        AFL_CHECK_SUCCEEDS(a("01. normal"), testee.evaluateString("a ${a} ${qq}", p));
    }

    // - no brace
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|3|7q");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("11. no brace"), testee.evaluateString("a $a $qq", p));
    }

    // - quoted
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|3 9");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("21. double quote"), testee.evaluateString("a \"${a} ${qq}\"", p));
    }

    // - single-quoted (no expansion)
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|${a} ${qq}");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("31. single quote"), testee.evaluateString("a '${a} ${qq}'", p));
    }

    // - braced (no expansion)
    {
        std::auto_ptr<afl::data::Value> p;
        mock.expectCall("a|${a} ${qq}");
        mock.provideReturnValue(mock.Success, 0);
        AFL_CHECK_SUCCEEDS(a("41. braced"), testee.evaluateString("a {${a} ${qq}}", p));
    }
    mock.checkFinish();
}

/** Test parser errors. These should not hit the CommandHandler. */
AFL_TEST("server.console.Parser:evaluateString:errors", a)
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock(a);
    server::console::Parser testee(env, term, fs, mock);
    std::auto_ptr<afl::data::Value> p;

    // No verb in pipe
    AFL_CHECK_THROWS(a("01. no verb"), testee.evaluateString("| x", p), std::exception);

    // Bad escapes
    AFL_CHECK_THROWS(a("11. bad escape"), testee.evaluateString("a \"\\""xX\"", p), std::exception);
    AFL_CHECK_THROWS(a("12. bad escape"), testee.evaluateString("a \"\\""x1X\"", p), std::exception);
    AFL_CHECK_THROWS(a("13. bad escape"), testee.evaluateString("a \"\\""uX\"", p), std::exception);
    AFL_CHECK_THROWS(a("14. bad escape"), testee.evaluateString("a \"\\""u1X\"", p), std::exception);
    AFL_CHECK_THROWS(a("15. bad escape"), testee.evaluateString("a \"\\""u11X\"", p), std::exception);
    AFL_CHECK_THROWS(a("16. bad escape"), testee.evaluateString("a \"\\""u111X\"", p), std::exception);

    // Missing file name
    AFL_CHECK_THROWS(a("21. missing file name"), testee.evaluateString("a <", p), std::exception);

    // Bad variable reference
    AFL_CHECK_THROWS(a("31. bad variable reference"), testee.evaluateString("a $$", p), std::exception);
    AFL_CHECK_THROWS(a("32. bad variable reference"), testee.evaluateString("a $ ", p), std::exception);
    AFL_CHECK_THROWS(a("33. bad variable reference"), testee.evaluateString("a $", p), std::exception);
    AFL_CHECK_THROWS(a("34. bad variable reference"), testee.evaluateString("a ${", p), std::exception);

    // Unpaired quotes
    AFL_CHECK_THROWS(a("41. missing quote"), testee.evaluateString("a 'x", p), std::exception);
    AFL_CHECK_THROWS(a("42. missing quote"), testee.evaluateString("a \"x", p), std::exception);
    AFL_CHECK_THROWS(a("43. missing quote"), testee.evaluateString("a {x", p), std::exception);
}

/** Test evaluateStringToBool(). */
AFL_TEST("server.console.Parser:evaluateStringToBool", a)
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock(a);
    server::console::Parser testee(env, term, fs, mock);

    // Null
    mock.expectCall("g|1");
    mock.provideReturnValue(mock.Success, 0);
    a.checkEqual("01. null", testee.evaluateStringToBool("g 1"), false);

    // Zero
    mock.expectCall("g|1");
    mock.provideReturnValue(mock.Success, server::makeIntegerValue(0));
    a.checkEqual("11. zero", testee.evaluateStringToBool("g 1"), false);

    // Nonzero
    mock.expectCall("g|1");
    mock.provideReturnValue(mock.Success, server::makeIntegerValue(3));
    a.checkEqual("21. nonzero", testee.evaluateStringToBool("g 1"), true);

    // Empty string
    mock.expectCall("g|1");
    mock.provideReturnValue(mock.Success, server::makeStringValue(""));
    a.checkEqual("31. empty", testee.evaluateStringToBool("g 1"), false);

    // Nonempty string
    mock.expectCall("g|1");
    mock.provideReturnValue(mock.Success, server::makeStringValue("q"));
    a.checkEqual("41. nonempty", testee.evaluateStringToBool("g 1"), true);

    mock.checkFinish();
}

/** Test piping with empty result. */
AFL_TEST("server.console.Parser:evaluateString:pipe:empty", a)
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock(a);
    server::console::Parser testee(env, term, fs, mock);

    // Pipe with empty array result
    mock.expectCall("a");
    mock.provideReturnValue(mock.Success, new afl::data::VectorValue(afl::data::Vector::create()));

    mock.expectCall("b|x");
    mock.provideReturnValue(mock.Success, server::makeIntegerValue(42));
    std::auto_ptr<afl::data::Value> p;
    AFL_CHECK_SUCCEEDS(a("01. eval"), testee.evaluateString("a | b x", p));
    a.checkEqual("02. result", server::toInteger(p.get()), 42);

    mock.checkFinish();
}

/** Test piping with different result types. */
AFL_TEST("server.console.Parser:evaluateString:pipe:typed", a)
{
    // Set up
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    ConsoleCommandHandlerMock mock(a);
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
    AFL_CHECK_SUCCEEDS(a("01. eval"), testee.evaluateString("bo|in|fl|st", p));
    a.checkEqual("02. result", server::toString(p.get()), "s");

    mock.checkFinish();
}
