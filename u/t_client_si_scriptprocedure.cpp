/**
  *  \file u/t_client_si_scriptprocedure.cpp
  *  \brief Test for client::si::ScriptProcedure
  */

#include "client/si/scriptprocedure.hpp"

#include "t_client_si.hpp"
#include "game/session.hpp"
#include "client/si/scriptside.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "afl/string/format.hpp"
#include "afl/data/stringvalue.hpp"

namespace {
    String_t theString;
    void theFunction(game::Session& /*session*/, client::si::ScriptSide& /*si*/, client::si::RequestLink1 link, interpreter::Arguments& args)
    {
        size_t numArgs = args.getNumArgs();
        afl::data::Value* firstArg = args.getNext();
        theString = afl::string::Format("pn=%s, argc=%d, arg1='%s'", link.getProcess().getName(), numArgs, interpreter::toString(firstArg, false));
    }
}

/** Test ScriptProcedure, normal case. */
void
TestClientSiScriptProcedure::testIt()
{
    // Make a dummy ScriptSide
    client::si::ScriptSide ss((util::RequestSender<client::si::UserSide>()));

    // Make a session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Make a ScriptProcedure
    client::si::ScriptProcedure testee(session, &ss, theFunction);

    // Verify basic properties
    TS_ASSERT(testee.isProcedureCall());
    TS_ASSERT_EQUALS(testee.getDimension(0), 0);
    TS_ASSERT_EQUALS(testee.getDimension(1), 0);
    TS_ASSERT_THROWS(testee.makeFirstContext(), interpreter::Error);
    TS_ASSERT_EQUALS(testee.toString(false).substr(0, 2), "#<");
    TS_ASSERT_EQUALS(testee.toString(true).substr(0, 2), "#<");

    // Make a process
    afl::sys::Log log;
    interpreter::World world(log, fs);
    interpreter::Process proc(world, "testIt", 12345);

    // Call it
    theString = "";
    afl::data::Segment seg;
    seg.pushBackNew(new afl::data::StringValue("hi"));
    testee.call(proc, seg, false);
    TS_ASSERT_EQUALS(theString, "pn=testIt, argc=1, arg1='hi'");
}

/** Test ScriptProcedure, null case (no ScriptSide). */
void
TestClientSiScriptProcedure::testNull()
{
    // Make a session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Make a ScriptProcedure
    client::si::ScriptProcedure testee(session, 0, theFunction);

    // Verify basic properties
    TS_ASSERT(testee.isProcedureCall());
    TS_ASSERT_EQUALS(testee.getDimension(0), 0);
    TS_ASSERT_EQUALS(testee.getDimension(1), 0);
    TS_ASSERT_THROWS(testee.makeFirstContext(), interpreter::Error);         // "Not iterable"
    TS_ASSERT_EQUALS(testee.toString(false).substr(0, 2), "#<");
    TS_ASSERT_EQUALS(testee.toString(true).substr(0, 2), "#<");

    // Make a process
    afl::sys::Log log;
    interpreter::World world(log, fs);
    interpreter::Process proc(world, "testIt", 12345);

    // Call it
    theString = "nope";
    afl::data::Segment seg;
    seg.pushBackNew(new afl::data::StringValue("hi"));
    TS_ASSERT_THROWS(testee.call(proc, seg, false), interpreter::Error);    // "Command not valid"
    TS_ASSERT_EQUALS(theString, "nope");
}
