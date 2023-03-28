/**
  *  \file u/t_game_interface_torpedofunction.cpp
  *  \brief Test for game::interface::TorpedoFunction
  */

#include "game/interface/torpedofunction.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** General tests. */
void
TestGameInterfaceTorpedoFunction::testIt()
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

    interpreter::test::ValueVerifier torpVerif(torpFunc, "torpFunc");
    torpVerif.verifyBasics();
    torpVerif.verifyNotSerializable();

    interpreter::test::ValueVerifier launVerif(launFunc, "launFunc");
    launVerif.verifyBasics();
    launVerif.verifyNotSerializable();

    TS_ASSERT_EQUALS(torpFunc.getDimension(0), 1);
    TS_ASSERT_EQUALS(torpFunc.getDimension(1), 6);

    // Test successful invocation
    {
        // Launcher
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(launFunc.get(args));
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "testIt: get").verifyInteger("COST.T", 1);
    }
    {
        // Torpedo
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(torpFunc.get(args));
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "testIt: get").verifyInteger("COST.T", 10);
    }

    // Test failing invocation
    {
        // arity error
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(torpFunc.get(args), interpreter::Error);
    }
    {
        // type error
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(torpFunc.get(args), interpreter::Error);
    }
    {
        // range error
        afl::data::Segment seg;
        seg.pushBackInteger(6);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(torpFunc.get(args), interpreter::Error);
    }

    // Test invocation with null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(torpFunc.get(args));
        TS_ASSERT(result.get() == 0);
    }

    // Test iteration
    {
        std::auto_ptr<interpreter::Context> result(torpFunc.makeFirstContext());
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "testIt: get").verifyInteger("ID", 3);
    }

    // Test set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(torpFunc.set(args, 0), interpreter::Error);
    }
}

/** Test empty session. */
void
TestGameInterfaceTorpedoFunction::testNull()
{
    // Empty session
    {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session(tx, fs);

        game::interface::TorpedoFunction testee(false, session);
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() == 0);
    }

    // Session populated with empty objects
    {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setShipList(new game::spec::ShipList());

        game::interface::TorpedoFunction testee(false, session);
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() == 0);
    }
}

