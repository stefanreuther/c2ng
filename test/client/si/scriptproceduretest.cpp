/**
  *  \file test/client/si/scriptproceduretest.cpp
  *  \brief Test for client::si::ScriptProcedure
  */

#include "client/si/scriptprocedure.hpp"

#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "client/si/scriptside.hpp"
#include "game/session.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"

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
AFL_TEST("client.si.ScriptProcedure:normal", a)
{
    // Make a session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Make a dummy ScriptSide
    client::si::ScriptSide ss((util::RequestSender<client::si::UserSide>()), session, new util::StopSignal());

    // Make a ScriptProcedure
    client::si::ScriptProcedure testee(session, &ss, theFunction);

    // Verify basic properties
    a.check("01. isProcedureCall", testee.isProcedureCall());
    a.checkEqual("02. getDimension", testee.getDimension(0), 0U);
    a.checkEqual("03. getDimension", testee.getDimension(1), 0U);
    AFL_CHECK_THROWS(a("04. makeFirstContext"), testee.makeFirstContext(), interpreter::Error);
    a.checkEqual("05. toString", testee.toString(false).substr(0, 2), "#<");
    a.checkEqual("06. toString", testee.toString(true).substr(0, 2), "#<");

    // Make a process
    afl::sys::Log log;
    interpreter::World world(log, tx, fs);
    interpreter::Process proc(world, "testIt", 12345);

    // Call it
    theString = "";
    afl::data::Segment seg;
    seg.pushBackNew(new afl::data::StringValue("hi"));
    static_cast<interpreter::CallableValue&>(testee).call(proc, seg, false);
    a.checkEqual("11. invocation result", theString, "pn=testIt, argc=1, arg1='hi'");
}

/** Test ScriptProcedure, null case (no ScriptSide). */
AFL_TEST("client.si.ScriptProcedure:null", a)
{
    // Make a session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Make a ScriptProcedure
    client::si::ScriptProcedure testee(session, 0, theFunction);

    // Verify basic properties
    a.check("01. isProcedureCall", testee.isProcedureCall());
    a.checkEqual("02. getDimension", testee.getDimension(0), 0U);
    a.checkEqual("03. getDimension", testee.getDimension(1), 0U);
    AFL_CHECK_THROWS(a("04. makeFirstContext"), testee.makeFirstContext(), interpreter::Error);         // "Not iterable"
    a.checkEqual("05. toString", testee.toString(false).substr(0, 2), "#<");
    a.checkEqual("06. toString", testee.toString(true).substr(0, 2), "#<");

    // Make a process
    afl::sys::Log log;
    interpreter::World world(log, tx, fs);
    interpreter::Process proc(world, "testIt", 12345);

    // Call it
    theString = "nope";
    afl::data::Segment seg;
    seg.pushBackNew(new afl::data::StringValue("hi"));
    AFL_CHECK_THROWS(a("11. invocation"), static_cast<interpreter::CallableValue&>(testee).call(proc, seg, false), interpreter::Error);    // "Command not valid"
    a.checkEqual("12. result unchanged", theString, "nope");
}
