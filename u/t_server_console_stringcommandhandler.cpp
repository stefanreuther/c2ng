/**
  *  \file u/t_server_console_stringcommandhandler.cpp
  *  \brief Test for server::console::StringCommandHandler
  */

#include <stdexcept>
#include "server/console/stringcommandhandler.hpp"

#include "t_server_console.hpp"
#include "server/console/environment.hpp"
#include "server/console/nullterminal.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "server/console/parser.hpp"
#include "server/types.hpp"

namespace {
    class NullCommandHandler : public server::console::CommandHandler {
     public:
        virtual bool call(const String_t& /*cmd*/, interpreter::Arguments /*args*/, server::console::Parser& /*parser*/, std::auto_ptr<afl::data::Value>& /*result*/)
            { return false; }
    };
}

/** Test "str". */
void
TestServerConsoleStringCommandHandler::testStr()
{
    // Environment
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    NullCommandHandler ch;
    server::console::Parser p(env, term, fs, ch);

    // Testee
    server::console::StringCommandHandler testee;

    // No arguments (too few)
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 0);
        TS_ASSERT_THROWS(testee.call("str", args, p, r), std::exception);
    }

    // Two arguments (too many)
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        TS_ASSERT_THROWS(testee.call("str", args, p, r), std::exception);
    }

    // One argument, null
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        TS_ASSERT_EQUALS(testee.call("str", args, p, r), true);
        TS_ASSERT_EQUALS(server::toString(r.get()), "");
    }

    // One argument, string
    {
        afl::data::Segment s;
        s.pushBackString("zz");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        TS_ASSERT_EQUALS(testee.call("str", args, p, r), true);
        TS_ASSERT_EQUALS(server::toString(r.get()), "zz");
    }

    // One argument, number
    {
        afl::data::Segment s;
        s.pushBackInteger(9);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        TS_ASSERT_EQUALS(testee.call("str", args, p, r), true);
        TS_ASSERT_EQUALS(server::toString(r.get()), "9");
    }
}

/** Test "str_eq". */
void
TestServerConsoleStringCommandHandler::testStrEq()
{
    // Environment
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    NullCommandHandler ch;
    server::console::Parser p(env, term, fs, ch);

    // Testee
    server::console::StringCommandHandler testee;

    // No arguments (too few)
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 0);
        TS_ASSERT_THROWS(testee.call("str_eq", args, p, r), std::exception);
    }

    // One arguments (too few)
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        TS_ASSERT_THROWS(testee.call("str_eq", args, p, r), std::exception);
    }

    // Three argument (too many)
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 3);
        TS_ASSERT_THROWS(testee.call("str_eq", args, p, r), std::exception);
    }

    // Equal
    {
        afl::data::Segment s;
        s.pushBackString("aaa");
        s.pushBackString("aaa");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        TS_ASSERT_EQUALS(testee.call("str_eq", args, p, r), true);
        TS_ASSERT_EQUALS(server::toInteger(r.get()), 1);
    }

    // Different
    {
        afl::data::Segment s;
        s.pushBackString("aaa");
        s.pushBackString("AAA");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        TS_ASSERT_EQUALS(testee.call("str_eq", args, p, r), true);
        TS_ASSERT_EQUALS(server::toInteger(r.get()), 0);
    }

    // Different
    {
        afl::data::Segment s;
        s.pushBackString("aaa");
        s.pushBackString("q");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        TS_ASSERT_EQUALS(testee.call("str_eq", args, p, r), true);
        TS_ASSERT_EQUALS(server::toInteger(r.get()), 0);
    }
}

/** Test "str_empty". */
void
TestServerConsoleStringCommandHandler::testStrEmpty()
{
    // Environment
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    NullCommandHandler ch;
    server::console::Parser p(env, term, fs, ch);

    // Testee
    server::console::StringCommandHandler testee;

    // No arguments
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 0);
        TS_ASSERT_EQUALS(testee.call("str_empty", args, p, r), true);
        TS_ASSERT_EQUALS(server::toInteger(r.get()), 1);
    }

    // Five null arguments
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 5);
        TS_ASSERT_EQUALS(testee.call("str_empty", args, p, r), true);
        TS_ASSERT_EQUALS(server::toInteger(r.get()), 1);
    }

    // Three empty string arguments
    {
        afl::data::Segment s;
        s.pushBackString("");
        s.pushBackString("");
        s.pushBackString("");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 3);
        TS_ASSERT_EQUALS(testee.call("str_empty", args, p, r), true);
        TS_ASSERT_EQUALS(server::toInteger(r.get()), 1);
    }

    // Three nonempty string arguments
    {
        afl::data::Segment s;
        s.pushBackString("a");
        s.pushBackString("b");
        s.pushBackString("c");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 3);
        TS_ASSERT_EQUALS(testee.call("str_empty", args, p, r), true);
        TS_ASSERT_EQUALS(server::toInteger(r.get()), 0);
    }

    // Three mixed string arguments
    {
        afl::data::Segment s;
        s.pushBackString("");
        s.pushBackString("b");
        s.pushBackString("");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 3);
        TS_ASSERT_EQUALS(testee.call("str_empty", args, p, r), true);
        TS_ASSERT_EQUALS(server::toInteger(r.get()), 0);
    }
}

/** Test errors. */
void
TestServerConsoleStringCommandHandler::testError()
{
    // Environment
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    NullCommandHandler ch;
    server::console::Parser p(env, term, fs, ch);

    // Testee
    server::console::StringCommandHandler testee;

    // Unrecognized command
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 0);
        TS_ASSERT_EQUALS(testee.call("str_fry", args, p, r), false);
    }
}

