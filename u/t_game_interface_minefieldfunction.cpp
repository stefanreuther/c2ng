/**
  *  \file u/t_game_interface_minefieldfunction.cpp
  *  \brief Test for game::interface::MinefieldFunction
  */

#include "game/interface/minefieldfunction.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/minefield.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

using game::map::Minefield;

/** General tests. */
void
TestGameInterfaceMinefieldFunction::testIt()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // - Root
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    session.setRoot(r.asPtr());

    // - Game
    afl::base::Ref<game::Game> g = *new game::Game();

    Minefield& mf = *g->currentTurn().universe().minefields().create(100);
    mf.addReport(game::map::Point(1200, 1300), 1, Minefield::IsWeb, Minefield::UnitsKnown, 400, 15, Minefield::MinefieldSwept);
    mf.internalCheck(15, r->hostVersion(), r->hostConfiguration());

    Minefield& mf2 = *g->currentTurn().universe().minefields().create(200);
    mf2.addReport(game::map::Point(2000, 4000), 2, Minefield::IsWeb, Minefield::UnitsKnown, 500, 15, Minefield::MinefieldSwept);
    mf2.internalCheck(15, r->hostVersion(), r->hostConfiguration());

    session.setGame(g.asPtr());

    // Test basic properties
    game::interface::MinefieldFunction testee(session);
    interpreter::test::ValueVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();

    TS_ASSERT_EQUALS(testee.getDimension(0), 1);
    TS_ASSERT_EQUALS(testee.getDimension(1), 201);

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(200);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "testIt: get").verifyInteger("ID", 200);
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
        // non-existant - does not throw in c2ng
        afl::data::Segment seg;
        seg.pushBackInteger(22222);
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
        interpreter::test::ContextVerifier(*result, "testIt: get").verifyInteger("ID", 100);
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
TestGameInterfaceMinefieldFunction::testNull()
{
    // Empty session
    {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session(tx, fs);

        game::interface::MinefieldFunction testee(session);
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() == 0);

        // No dimension because no game
        TS_ASSERT_EQUALS(testee.getDimension(1), 0);
    }

    // Session populated with empty objects
    {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setGame(new game::Game());

        game::interface::MinefieldFunction testee(session);
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() == 0);

        // Slot 0 is present (but empty)
        TS_ASSERT_EQUALS(testee.getDimension(1), 1);
    }
}

