/**
  *  \file u/t_game_interface_playerfunction.cpp
  *  \brief Test for game::interface::PlayerFunction
  */

#include "game/interface/playerfunction.hpp"

#include "t_game_interface.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/player.hpp"
#include "game/playerlist.hpp"
#include "game/test/root.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** General tests. */
void
TestGameInterfacePlayerFunction::testIt()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());

    session.getRoot()->playerList().create(3)->setName(game::Player::ShortName, "Three");
    session.getRoot()->playerList().create(5)->setName(game::Player::ShortName, "Five");

    // Test basic properties
    game::interface::PlayerFunction testee(session);
    interpreter::test::ValueVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();

    TS_ASSERT_EQUALS(testee.getDimension(0), 1);
    TS_ASSERT_EQUALS(testee.getDimension(1), 6);

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "testIt: get").verifyInteger("RACE$", 3);
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
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() == 0);
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
        interpreter::test::ContextVerifier(*result, "testIt: iterate").verifyInteger("RACE$", 3);
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
TestGameInterfacePlayerFunction::testEmpty()
{
    // Empty session
    {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session(tx, fs);

        game::interface::PlayerFunction testee(session);
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() == 0);
    }

    // Session populated with empty objects
    {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setGame(new game::Game());

        game::interface::PlayerFunction testee(session);
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() == 0);
    }
}

