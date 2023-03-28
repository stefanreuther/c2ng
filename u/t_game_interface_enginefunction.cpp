/**
  *  \file u/t_game_interface_enginefunction.cpp
  *  \brief Test for game::interface::EngineFunction
  */

#include "game/interface/enginefunction.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** General tests. */
void
TestGameInterfaceEngineFunction::testIt()
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
    interpreter::test::ValueVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();

    TS_ASSERT_EQUALS(testee.getDimension(0), 1);
    TS_ASSERT_EQUALS(testee.getDimension(1), 4);

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "testIt: get").verifyInteger("ID", 3);
    }

    // Test failing invocation
    {
        // arity error
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }
    {
        // type error
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }
    {
        // range error
        afl::data::Segment seg;
        seg.pushBackInteger(6);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }

    // Test invocation with null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() == 0);
    }

    // Test iteration
    {
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "testIt: get").verifyInteger("ID", 2);
    }

    // Test set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.set(args, 0), interpreter::Error);
    }
}

/** Test empty session. */
void
TestGameInterfaceEngineFunction::testNull()
{
    // Empty session
    {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session(tx, fs);

        game::interface::EngineFunction testee(session);
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() == 0);
    }

    // Session populated with empty objects
    {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session(tx, fs);
        session.setShipList(new game::spec::ShipList());

        game::interface::EngineFunction testee(session);
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() == 0);
    }
}

