/**
  *  \file u/t_game_interface_ufofunction.cpp
  *  \brief Test for game::interface::UfoFunction
  */

#include "game/interface/ufofunction.hpp"

#include "t_game_interface.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** General tests. */
void
TestGameInterfaceUfoFunction::testIt()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    session.setGame(new game::Game());
    game::Turn& turn = session.getGame()->currentTurn();

    turn.universe().ufos().addUfo(51, 1, 2)->setColorCode(10);
    turn.universe().ufos().addUfo(77, 1, 2)->setColorCode(20);

    // Testee
    game::interface::UfoFunction testee(session);
    interpreter::test::ValueVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();
    TS_ASSERT_EQUALS(testee.getDimension(0), 1);
    TS_ASSERT_LESS_THAN_EQUALS(10000, testee.getDimension(1));

    // Successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(77);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> ctx(testee.get(args));
        TS_ASSERT(ctx.get() != 0);
        interpreter::test::ContextVerifier(*ctx, "(77)").verifyInteger("ID", 77);
    }

    // Invoke with unknown Id
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> ctx(testee.get(args));
        TS_ASSERT(ctx.get() == 0);
    }

    // Invoke with null
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> ctx(testee.get(args));
        TS_ASSERT(ctx.get() == 0);
    }

    // Arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackString("77");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }

    // Set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(77);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.set(args, 0), interpreter::Error);
    }

    // Iteration
    {
        std::auto_ptr<interpreter::Context> ctx(testee.makeFirstContext());
        TS_ASSERT(ctx.get() != 0);
        interpreter::test::ContextVerifier(*ctx, "first").verifyInteger("ID", 51);
    }
}

/** Tests on empty session. */
void
TestGameInterfaceUfoFunction::testNull()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;

    // No game
    {
        game::Session session(tx, fs);
        game::interface::UfoFunction testee(session);

        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> ctx(testee.get(args));
        TS_ASSERT(ctx.get() == 0);

        ctx.reset(testee.makeFirstContext());
        TS_ASSERT(ctx.get() == 0);
    }

    // No objects
    {
        game::Session session(tx, fs);
        session.setGame(new game::Game());
        game::interface::UfoFunction testee(session);

        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> ctx(testee.get(args));
        TS_ASSERT(ctx.get() == 0);

        ctx.reset(testee.makeFirstContext());
        TS_ASSERT(ctx.get() == 0);
    }
}

