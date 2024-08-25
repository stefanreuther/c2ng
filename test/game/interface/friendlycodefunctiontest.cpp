/**
  *  \file test/game/interface/friendlycodefunctiontest.cpp
  *  \brief Test for game::interface::FriendlyCodeFunction
  */

#include "game/interface/friendlycodefunction.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** Test general behaviour. */
AFL_TEST("game.interface.FriendlyCodeFunction:basics", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.getShipList()->friendlyCodes().addCode(game::spec::FriendlyCode("abc", ",one", session.translator()));
    session.getShipList()->friendlyCodes().addCode(game::spec::FriendlyCode("xyz", ",two", session.translator()));

    // Test basic properties
    game::interface::FriendlyCodeFunction testee(session);
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

    a.checkEqual("01. getDimension", testee.getDimension(0), 0U);

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackString("xyz");
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNonNull("11. get 'xyz'", result.get());
        interpreter::test::ContextVerifier(*result, a("21. get 'xyz'")).verifyString("DESCRIPTION", "two");
    }

    // Invocation with null
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNull("21. get null", result.get());
    }

    // Invocation with unknown value
    {
        afl::data::Segment seg;
        seg.pushBackString("pqr");
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNull("31. get unknown", result.get());
    }

    // Test failing invocation: arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("41. arity error"), testee.get(args), interpreter::Error);
    }

    // Cannot assign 'FriendlyCode("xyz") := ...'
    {
        afl::data::Segment seg;
        seg.pushBackString("xyz");
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("51. set"), testee.set(args, 0), interpreter::Error);
    }

    // Iteration
    {
        std::auto_ptr<interpreter::Context> p(testee.makeFirstContext());
        a.checkNonNull("61. makeFirstContext", p.get());
        interpreter::test::ContextVerifier(*p, a("62. makeFirstContext")).verifyString("DESCRIPTION", "one");
    }
}

// No root
AFL_TEST("game.interface.FriendlyCodeFunction:create:no-root", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.getShipList()->friendlyCodes().addCode(game::spec::FriendlyCode("abc", ",one", session.translator()));

    game::interface::FriendlyCodeFunction testee(session);

    // Invocation
    afl::data::Segment seg;
    seg.pushBackString("xyz");
    interpreter::Arguments args(seg, 0, 1);
    std::auto_ptr<interpreter::Context> result(testee.get(args));
    a.checkNull("get", result.get());

    // Iteration
    result.reset(testee.makeFirstContext());
    a.checkNull("makeFirstContext", result.get());
}

// No ship list
AFL_TEST("game.interface.FriendlyCodeFunction:create:no-shiplist", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());

    game::interface::FriendlyCodeFunction testee(session);

    // Invocation
    afl::data::Segment seg;
    seg.pushBackString("xyz");
    interpreter::Arguments args(seg, 0, 1);
    std::auto_ptr<interpreter::Context> result(testee.get(args));
    a.checkNull("get", result.get());

    // Iteration
    result.reset(testee.makeFirstContext());
    a.checkNull("makeFirstContext", result.get());
}
