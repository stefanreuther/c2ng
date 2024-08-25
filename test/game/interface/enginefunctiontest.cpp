/**
  *  \file test/game/interface/enginefunctiontest.cpp
  *  \brief Test for game::interface::EngineFunction
  */

#include "game/interface/enginefunction.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** General tests. */
AFL_TEST("game.interface.EngineFunction:basics", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.getShipList()->engines().create(2)->setName("E2");
    session.getShipList()->engines().create(3)->setName("E3");

    // Test basic properties
    game::interface::EngineFunction testee(session);
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

    a.checkEqual("01. getDimension 0", testee.getDimension(0), 1U);
    a.checkEqual("02. getDimension 1", testee.getDimension(1), 4U);

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNonNull("11. success", result.get());
        interpreter::test::ContextVerifier(*result, a("12. get")).verifyInteger("ID", 3);
    }

    // Test failing invocation
    {
        // arity error
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("21. arity error"), testee.get(args), interpreter::Error);
    }
    {
        // type error
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("22. type error"), testee.get(args), interpreter::Error);
    }
    {
        // range error
        afl::data::Segment seg;
        seg.pushBackInteger(6);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("23. range error"), testee.get(args), interpreter::Error);
    }

    // Test invocation with null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNull("31. null", result.get());
    }

    // Test iteration
    {
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        a.checkNonNull("41. makeFirstContext", result.get());
        interpreter::test::ContextVerifier(*result, a("42. makeFirstContext")).verifyInteger("ID", 2);
    }

    // Test set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("51. set"), testee.set(args, 0), interpreter::Error);
    }
}

// Empty session
AFL_TEST("game.interface.EngineFunction:empty-session", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    game::interface::EngineFunction testee(session);
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    a.checkNull("", result.get());
}

// Session populated with empty objects
AFL_TEST("game.interface.EngineFunction:empty-shiplist", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());

    game::interface::EngineFunction testee(session);
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    a.checkNull("", result.get());
}
