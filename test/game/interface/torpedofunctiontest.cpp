/**
  *  \file test/game/interface/torpedofunctiontest.cpp
  *  \brief Test for game::interface::TorpedoFunction
  */

#include "game/interface/torpedofunction.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** General tests. */
AFL_TEST("game.interface.TorpedoFunction:basics", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());

    game::spec::TorpedoLauncher& t3 = *session.getShipList()->launchers().create(3);
    t3.setName("Three");
    t3.cost().set(game::spec::Cost::Tritanium, 1);
    t3.torpedoCost().set(game::spec::Cost::Tritanium, 10);

    game::spec::TorpedoLauncher& t5 = *session.getShipList()->launchers().create(5);
    t5.setName("Five");
    t5.cost().set(game::spec::Cost::Tritanium, 7);
    t5.torpedoCost().set(game::spec::Cost::Tritanium, 17);

    // Test basic properties
    game::interface::TorpedoFunction torpFunc(false, session);
    game::interface::TorpedoFunction launFunc(true, session);

    interpreter::test::ValueVerifier torpVerif(torpFunc, a("torpFunc"));
    torpVerif.verifyBasics();
    torpVerif.verifyNotSerializable();

    interpreter::test::ValueVerifier launVerif(launFunc, a("launFunc"));
    launVerif.verifyBasics();
    launVerif.verifyNotSerializable();

    a.checkEqual("01. getDimension 0", torpFunc.getDimension(0), 1);
    a.checkEqual("02. getDimension 1", torpFunc.getDimension(1), 6);

    // Test successful invocation
    {
        // Launcher
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(launFunc.get(args));
        a.checkNonNull("11. Launcher()", result.get());
        interpreter::test::ContextVerifier(*result, a("Launcher")).verifyInteger("COST.T", 1);
    }
    {
        // Torpedo
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(torpFunc.get(args));
        a.checkNonNull("12. Torpedo()", result.get());
        interpreter::test::ContextVerifier(*result, a("Torpedo")).verifyInteger("COST.T", 10);
    }

    // Test failing invocation
    {
        // arity error
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("21. arity error"), torpFunc.get(args), interpreter::Error);
    }
    {
        // type error
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("22. type error"), torpFunc.get(args), interpreter::Error);
    }
    {
        // range error
        afl::data::Segment seg;
        seg.pushBackInteger(6);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("23. range error"), torpFunc.get(args), interpreter::Error);
    }

    // Test invocation with null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(torpFunc.get(args));
        a.checkNull("31. null", result.get());
    }

    // Test iteration
    {
        std::auto_ptr<interpreter::Context> result(torpFunc.makeFirstContext());
        a.checkNonNull("41. makeFirstContext", result.get());
        interpreter::test::ContextVerifier(*result, a("42. makeFirstContext")).verifyInteger("ID", 3);
    }

    // Test set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("51. set"), torpFunc.set(args, 0), interpreter::Error);
    }
}

// Empty session
AFL_TEST("game.interface.TorpedoFunction:makeFirstContext:empty-session", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    game::interface::TorpedoFunction testee(false, session);
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    a.checkNull("makeFirstContext", result.get());
}

// Session populated with empty objects
AFL_TEST("game.interface.TorpedoFunction:makeFirstContext:empty-shiplist", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());

    game::interface::TorpedoFunction testee(false, session);
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    a.checkNull("makeFirstContext", result.get());
}
