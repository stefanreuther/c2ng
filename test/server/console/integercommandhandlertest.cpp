/**
  *  \file test/server/console/integercommandhandlertest.cpp
  *  \brief Test for server::console::IntegerCommandHandler
  */

#include "server/console/integercommandhandler.hpp"

#include "afl/data/access.hpp"
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

/** Test "int". */
AFL_TEST("server.console.IntegerCommandHandler:int", a)
{
    // Environment
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    NullCommandHandler ch;
    server::console::Parser p(env, term, fs, ch);

    // Testee
    server::console::IntegerCommandHandler testee;

    // No arguments (too few)
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 0);
        AFL_CHECK_THROWS(a("01. too few args"), testee.call("int", args, p, r), std::exception);
    }

    // Two arguments (too many)
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        AFL_CHECK_THROWS(a("11. too many args"), testee.call("int", args, p, r), std::exception);
    }

    // One argument, null: stays null
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        a.checkEqual("21. null", testee.call("int", args, p, r), true);
        a.checkNull("22. null", r.get());
    }

    // One argument, string
    {
        afl::data::Segment s;
        s.pushBackString("42");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        a.checkEqual("31. string", testee.call("int", args, p, r), true);
        a.checkEqual("32. string", server::toInteger(r.get()), 42);
    }

    // One argument, number
    {
        afl::data::Segment s;
        s.pushBackInteger(192);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        a.checkEqual("41. num", testee.call("int", args, p, r), true);
        a.checkEqual("42. num", server::toInteger(r.get()), 192);
    }

    // One argument, unparseable string: null
    {
        afl::data::Segment s;
        s.pushBackString("huh?");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        a.checkEqual("51. bad-string", testee.call("int", args, p, r), true);
        a.checkNull("52. bad-string", r.get());
    }
}

/** Test "int_not". */
AFL_TEST("server.console.IntegerCommandHandler:int_not", a)
{
    // Environment
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    NullCommandHandler ch;
    server::console::Parser p(env, term, fs, ch);

    // Testee
    server::console::IntegerCommandHandler testee;

    // No arguments (too few)
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 0);
        AFL_CHECK_THROWS(a("01. too few args"), testee.call("int_not", args, p, r), std::exception);
    }

    // Two arguments (too many)
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        AFL_CHECK_THROWS(a("11. too many args"), testee.call("int_not", args, p, r), std::exception);
    }

    // One argument, null: produces 0
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        a.checkEqual("21. null", testee.call("int_not", args, p, r), true);
        a.checkNull("22. null", r.get());
    }

    // One argument, string
    {
        afl::data::Segment s;
        s.pushBackString("7");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        a.checkEqual("31. string", testee.call("int_not", args, p, r), true);
        a.checkEqual("32. string", server::toInteger(r.get()), 0);
    }

    // One argument, integer
    {
        afl::data::Segment s;
        s.pushBackInteger(0);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        a.checkEqual("41. int", testee.call("int_not", args, p, r), true);
        a.checkEqual("42. int", server::toInteger(r.get()), 1);
    }
}

/** Test "int_add". */
AFL_TEST("server.console.IntegerCommandHandler:int_add", a)
{
    // Environment
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    NullCommandHandler ch;
    server::console::Parser p(env, term, fs, ch);

    // Testee
    server::console::IntegerCommandHandler testee;

    // No arguments
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 0);
        a.checkEqual("01. no args", testee.call("int_add", args, p, r), true);
        a.checkNonNull("02. no args", r.get());
        a.checkEqual("03. no args", server::toInteger(r.get()), 0);
    }

    // Some arguments
    {
        afl::data::Segment s;
        s.pushBackString("10");
        s.pushBackInteger(7);
        s.pushBackString("200");
        s.pushBackInteger(4000);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 4);
        a.checkEqual("11. some args", testee.call("int_add", args, p, r), true);
        a.checkEqual("12. some args", server::toInteger(r.get()), 4217);
    }

    // Unparseable arguments
    {
        afl::data::Segment s;
        s.pushBackString("10");
        s.pushBackInteger(7);
        s.pushBackString("boo!");
        s.pushBackInteger(4000);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 4);
        AFL_CHECK_THROWS(a("21. error"), testee.call("int_add", args, p, r), std::exception);
    }
}

/** Test "int_seq". */
AFL_TEST("server.console.IntegerCommandHandler:int_seq", a)
{
    // Environment
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    NullCommandHandler ch;
    server::console::Parser p(env, term, fs, ch);

    // Testee
    server::console::IntegerCommandHandler testee;

    // No arguments
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 0);
        AFL_CHECK_THROWS(a("01. too few args"), testee.call("int_seq", args, p, r), std::exception);
    }

    // Too many arguments
    {
        afl::data::Segment s;
        s.pushBackInteger(1);
        s.pushBackInteger(5);
        s.pushBackInteger(9);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 3);
        AFL_CHECK_THROWS(a("11. too many args"), testee.call("int_seq", args, p, r), std::exception);
    }

    // Normal case
    {
        afl::data::Segment s;
        s.pushBackInteger(2);
        s.pushBackInteger(5);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        a.checkEqual("21. normal", testee.call("int_seq", args, p, r), true);
        a.checkEqual("22. result", afl::data::Access(r.get()).getArraySize(), 4U);
        a.checkEqual("23. result", afl::data::Access(r.get())[0].toInteger(), 2);
        a.checkEqual("24. result", afl::data::Access(r.get())[1].toInteger(), 3);
        a.checkEqual("25. result", afl::data::Access(r.get())[2].toInteger(), 4);
        a.checkEqual("26. result", afl::data::Access(r.get())[3].toInteger(), 5);
    }

    // Border case
    {
        afl::data::Segment s;
        s.pushBackInteger(5);
        s.pushBackInteger(5);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        a.checkEqual("31. unit", testee.call("int_seq", args, p, r), true);
        a.checkEqual("32. result", afl::data::Access(r.get()).getArraySize(), 1U);
        a.checkEqual("33. result", afl::data::Access(r.get())[0].toInteger(), 5);
    }

    // Denormal case
    {
        afl::data::Segment s;
        s.pushBackInteger(6);
        s.pushBackInteger(5);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        a.checkEqual("41. empty", testee.call("int_seq", args, p, r), true);
        a.checkNonNull("42. result", r.get());
        a.checkEqual("43. result", afl::data::Access(r.get()).getArraySize(), 0U);
    }
}

/** Test errors. */
AFL_TEST("server.console.IntegerCommandHandler:error", a)
{
    // Environment
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    NullCommandHandler ch;
    server::console::Parser p(env, term, fs, ch);

    // Testee
    server::console::IntegerCommandHandler testee;

    // Unrecognized command
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 0);
        a.checkEqual("01. unrecognized", testee.call("int_fry", args, p, r), false);
    }
}
