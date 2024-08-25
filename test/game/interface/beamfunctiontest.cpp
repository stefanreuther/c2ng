/**
  *  \file test/game/interface/beamfunctiontest.cpp
  *  \brief Test for game::interface::BeamFunction
  */

#include "game/interface/beamfunction.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** General tests. */
AFL_TEST("game.interface.BeamFunction:general", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.getShipList()->beams().create(3)->setName("Three");
    session.getShipList()->beams().create(5)->setName("Five");

    // Test basic properties
    game::interface::BeamFunction testee(session);
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

    a.checkEqual("01. getDimension", testee.getDimension(0), 1U);
    a.checkEqual("02. getDimension", testee.getDimension(1), 6U);

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNonNull("11. result", result.get());
        interpreter::test::ContextVerifier(*result, a("12. get")).verifyInteger("ID", 3);
    }

    // Test failing invocation
    {
        // arity error
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("21. get"), testee.get(args), interpreter::Error);
    }
    {
        // type error
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("22. get"), testee.get(args), interpreter::Error);
    }
    {
        // range error
        afl::data::Segment seg;
        seg.pushBackInteger(6);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("23. get"), testee.get(args), interpreter::Error);
    }

    // Test invocation with null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNull("31. get", result.get());
    }

    // Test iteration
    {
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        a.checkNonNull("41. makeFirstContext", result.get());
        interpreter::test::ContextVerifier(*result, a("42. get")).verifyInteger("ID", 3);
    }

    // Test set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("51. set"), testee.set(args, 0), interpreter::Error);
    }
}

/** Test empty session. */
AFL_TEST("game.interface.BeamFunction:null", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;

    // Empty session
    {
        game::Session session(tx, fs);

        game::interface::BeamFunction testee(session);
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        a.checkNull("01. get", result.get());
    }

    // Session populated with empty objects
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setShipList(new game::spec::ShipList());

        game::interface::BeamFunction testee(session);
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        a.checkNull("11. get", result.get());
    }
}
