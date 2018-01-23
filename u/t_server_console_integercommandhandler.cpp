/**
  *  \file u/t_server_console_integercommandhandler.cpp
  *  \brief Test for server::console::IntegerCommandHandler
  */

#include <stdexcept>
#include "server/console/integercommandhandler.hpp"

#include "t_server_console.hpp"
#include "server/console/environment.hpp"
#include "server/console/nullterminal.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "server/console/parser.hpp"
#include "server/types.hpp"
#include "afl/data/access.hpp"

namespace {
    class NullCommandHandler : public server::console::CommandHandler {
     public:
        virtual bool call(const String_t& /*cmd*/, interpreter::Arguments /*args*/, server::console::Parser& /*parser*/, std::auto_ptr<afl::data::Value>& /*result*/)
            { return false; }
    };
}

/** Test "int". */
void
TestServerConsoleIntegerCommandHandler::testInt()
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
        TS_ASSERT_THROWS(testee.call("int", args, p, r), std::exception);
    }

    // Two arguments (too many)
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        TS_ASSERT_THROWS(testee.call("int", args, p, r), std::exception);
    }

    // One argument, null: stays null
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        TS_ASSERT_EQUALS(testee.call("int", args, p, r), true);
        TS_ASSERT(r.get() == 0);
    }

    // One argument, string
    {
        afl::data::Segment s;
        s.pushBackString("42");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        TS_ASSERT_EQUALS(testee.call("int", args, p, r), true);
        TS_ASSERT_EQUALS(server::toInteger(r.get()), 42);
    }

    // One argument, number
    {
        afl::data::Segment s;
        s.pushBackInteger(192);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        TS_ASSERT_EQUALS(testee.call("int", args, p, r), true);
        TS_ASSERT_EQUALS(server::toInteger(r.get()), 192);
    }

    // One argument, unparseable string: null
    {
        afl::data::Segment s;
        s.pushBackString("huh?");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        TS_ASSERT_EQUALS(testee.call("int", args, p, r), true);
        TS_ASSERT(r.get() == 0);
    }
}

/** Test "int_not". */
void
TestServerConsoleIntegerCommandHandler::testIntNot()
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
        TS_ASSERT_THROWS(testee.call("int_not", args, p, r), std::exception);
    }

    // Two arguments (too many)
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        TS_ASSERT_THROWS(testee.call("int_not", args, p, r), std::exception);
    }

    // One argument, null: produces 0
    {
        afl::data::Segment s;
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        TS_ASSERT_EQUALS(testee.call("int_not", args, p, r), true);
        TS_ASSERT(r.get() == 0);
    }

    // One argument, string
    {
        afl::data::Segment s;
        s.pushBackString("7");
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        TS_ASSERT_EQUALS(testee.call("int_not", args, p, r), true);
        TS_ASSERT_EQUALS(server::toInteger(r.get()), 0);
    }

    // One argument, integer
    {
        afl::data::Segment s;
        s.pushBackInteger(0);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 1);
        TS_ASSERT_EQUALS(testee.call("int_not", args, p, r), true);
        TS_ASSERT_EQUALS(server::toInteger(r.get()), 1);
    }
}

/** Test "int_add". */
void
TestServerConsoleIntegerCommandHandler::testIntAdd()
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
        TS_ASSERT_EQUALS(testee.call("int_add", args, p, r), true);
        TS_ASSERT(r.get() != 0);
        TS_ASSERT_EQUALS(server::toInteger(r.get()), 0);
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
        TS_ASSERT_EQUALS(testee.call("int_add", args, p, r), true);
        TS_ASSERT_EQUALS(server::toInteger(r.get()), 4217);
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
        TS_ASSERT_THROWS(testee.call("int_add", args, p, r), std::exception);
    }
}

/** Test "int_seq". */
void
TestServerConsoleIntegerCommandHandler::testIntSeq()
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
        TS_ASSERT_THROWS(testee.call("int_seq", args, p, r), std::exception);
    }

    // Too many arguments
    {
        afl::data::Segment s;
        s.pushBackInteger(1);
        s.pushBackInteger(5);
        s.pushBackInteger(9);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 3);
        TS_ASSERT_THROWS(testee.call("int_seq", args, p, r), std::exception);
    }

    // Normal case
    {
        afl::data::Segment s;
        s.pushBackInteger(2);
        s.pushBackInteger(5);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        TS_ASSERT_EQUALS(testee.call("int_seq", args, p, r), true);
        TS_ASSERT_EQUALS(afl::data::Access(r.get()).getArraySize(), 4U);
        TS_ASSERT_EQUALS(afl::data::Access(r.get())[0].toInteger(), 2);
        TS_ASSERT_EQUALS(afl::data::Access(r.get())[1].toInteger(), 3);
        TS_ASSERT_EQUALS(afl::data::Access(r.get())[2].toInteger(), 4);
        TS_ASSERT_EQUALS(afl::data::Access(r.get())[3].toInteger(), 5);
    }

    // Border case
    {
        afl::data::Segment s;
        s.pushBackInteger(5);
        s.pushBackInteger(5);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        TS_ASSERT_EQUALS(testee.call("int_seq", args, p, r), true);
        TS_ASSERT_EQUALS(afl::data::Access(r.get()).getArraySize(), 1U);
        TS_ASSERT_EQUALS(afl::data::Access(r.get())[0].toInteger(), 5);
    }

    // Denormal case
    {
        afl::data::Segment s;
        s.pushBackInteger(6);
        s.pushBackInteger(5);
        std::auto_ptr<afl::data::Value> r;
        interpreter::Arguments args(s, 0, 2);
        TS_ASSERT_EQUALS(testee.call("int_seq", args, p, r), true);
        TS_ASSERT(r.get() != 0);
        TS_ASSERT_EQUALS(afl::data::Access(r.get()).getArraySize(), 0U);
    }
}

/** Test errors. */
void
TestServerConsoleIntegerCommandHandler::testError()
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
        TS_ASSERT_EQUALS(testee.call("int_fry", args, p, r), false);
    }
}

