/**
  *  \file test/server/console/stringcommandhandlertest.cpp
  *  \brief Test for server::console::StringCommandHandler
  */

#include "server/console/stringcommandhandler.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/test/testrunner.hpp"
#include "server/console/environment.hpp"
#include "server/console/nullterminal.hpp"
#include "server/console/parser.hpp"
#include "server/types.hpp"
#include <stdexcept>

namespace {
    class NullCommandHandler : public server::console::CommandHandler {
     public:
        virtual bool call(const String_t& /*cmd*/, interpreter::Arguments /*args*/, server::console::Parser& /*parser*/, std::auto_ptr<afl::data::Value>& /*result*/)
            { return false; }
    };
}

/** Test "str". */
AFL_TEST("server.console.StringCommandHandler:str", a)
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
        AFL_CHECK_THROWS(a("01. no args"), testee.call("str", args, p, r), std::exception);
    }

    // Two arguments (too many)
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        AFL_CHECK_THROWS(a("11. too many args"), testee.call("str", args, p, r), std::exception);
    }

    // One argument, null
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        a.checkEqual("21. null", testee.call("str", args, p, r), true);
        a.checkEqual("22. result", server::toString(r.get()), "");
    }

    // One argument, string
    {
        afl::data::Segment s;
        s.pushBackString("zz");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        a.checkEqual("31. string", testee.call("str", args, p, r), true);
        a.checkEqual("32. result", server::toString(r.get()), "zz");
    }

    // One argument, number
    {
        afl::data::Segment s;
        s.pushBackInteger(9);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        a.checkEqual("41. int", testee.call("str", args, p, r), true);
        a.checkEqual("42. result", server::toString(r.get()), "9");
    }
}

/** Test "str_eq". */
AFL_TEST("server.console.StringCommandHandler:str_eq", a)
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
        AFL_CHECK_THROWS(a("01. no args"), testee.call("str_eq", args, p, r), std::exception);
    }

    // One arguments (too few)
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        AFL_CHECK_THROWS(a("11. too few args"), testee.call("str_eq", args, p, r), std::exception);
    }

    // Three argument (too many)
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 3);
        AFL_CHECK_THROWS(a("21. too many args"), testee.call("str_eq", args, p, r), std::exception);
    }

    // Equal
    {
        afl::data::Segment s;
        s.pushBackString("aaa");
        s.pushBackString("aaa");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        a.checkEqual("31. equal", testee.call("str_eq", args, p, r), true);
        a.checkEqual("32. result", server::toInteger(r.get()), 1);
    }

    // Different
    {
        afl::data::Segment s;
        s.pushBackString("aaa");
        s.pushBackString("AAA");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        a.checkEqual("41. different", testee.call("str_eq", args, p, r), true);
        a.checkEqual("42. result", server::toInteger(r.get()), 0);
    }

    // Different
    {
        afl::data::Segment s;
        s.pushBackString("aaa");
        s.pushBackString("q");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        a.checkEqual("51. different", testee.call("str_eq", args, p, r), true);
        a.checkEqual("52. result", server::toInteger(r.get()), 0);
    }
}

/** Test "str_empty". */
AFL_TEST("server.console.StringCommandHandler:str_empty", a)
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
        a.checkEqual("01. no args", testee.call("str_empty", args, p, r), true);
        a.checkEqual("02. result", server::toInteger(r.get()), 1);
    }

    // Five null arguments
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 5);
        a.checkEqual("11. null args", testee.call("str_empty", args, p, r), true);
        a.checkEqual("12. result", server::toInteger(r.get()), 1);
    }

    // Three empty string arguments
    {
        afl::data::Segment s;
        s.pushBackString("");
        s.pushBackString("");
        s.pushBackString("");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 3);
        a.checkEqual("21. empty args", testee.call("str_empty", args, p, r), true);
        a.checkEqual("22. result", server::toInteger(r.get()), 1);
    }

    // Three nonempty string arguments
    {
        afl::data::Segment s;
        s.pushBackString("a");
        s.pushBackString("b");
        s.pushBackString("c");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 3);
        a.checkEqual("31. nonempty args", testee.call("str_empty", args, p, r), true);
        a.checkEqual("32. result", server::toInteger(r.get()), 0);
    }

    // Three mixed string arguments
    {
        afl::data::Segment s;
        s.pushBackString("");
        s.pushBackString("b");
        s.pushBackString("");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 3);
        a.checkEqual("41. mixed args", testee.call("str_empty", args, p, r), true);
        a.checkEqual("42. result", server::toInteger(r.get()), 0);
    }
}

/** Test errors. */
AFL_TEST("server.console.StringCommandHandler:error", a)
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
        a.checkEqual("01. unrecognized", testee.call("str_fry", args, p, r), false);
    }
}
