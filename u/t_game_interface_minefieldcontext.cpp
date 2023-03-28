/**
  *  \file u/t_game_interface_minefieldcontext.cpp
  *  \brief Test for game::interface::MinefieldContext
  */

#include "game/interface/minefieldcontext.hpp"

#include "t_game_interface.hpp"
#include "game/test/root.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/map/minefield.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "game/turn.hpp"
#include "afl/string/nulltranslator.hpp"
#include "interpreter/error.hpp"
#include "afl/sys/log.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "interpreter/world.hpp"
#include "interpreter/process.hpp"
#include "afl/data/segment.hpp"
#include "interpreter/callablevalue.hpp"
#include "game/session.hpp"

using game::Player;
using game::map::Minefield;

/** Test basics: general behaviour, specific properties. */
void
TestGameInterfaceMinefieldContext::testBasics()
{
    // Environment
    const int PLAYER_NR = 9;
    const int MINEFIELD_NR = 77;

    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    Player& p = *root->playerList().create(PLAYER_NR);
    p.setName(Player::LongName, "Long Nine");
    p.setName(Player::ShortName, "Short Nine");
    p.setName(Player::AdjectiveName, "nine");

    afl::base::Ref<game::Game> g = *new game::Game();
    Minefield& mf = *g->currentTurn().universe().minefields().create(MINEFIELD_NR);
    mf.addReport(game::map::Point(1200, 1300), PLAYER_NR, Minefield::IsWeb, Minefield::UnitsKnown, 400, 15, Minefield::MinefieldSwept);
    mf.internalCheck(15, root->hostVersion(), root->hostConfiguration());

    afl::string::NullTranslator tx;

    // Instance
    game::interface::MinefieldContext testee(MINEFIELD_NR, root, g, tx);
    interpreter::test::ContextVerifier verif(testee, "testBasics");
    verif.verifyBasics();
    verif.verifySerializable(interpreter::TagNode::Tag_Minefield, MINEFIELD_NR, afl::base::Nothing);
    verif.verifyTypes();
    TS_ASSERT(testee.getObject() == &mf);

    // Specific properties
    TS_ASSERT_EQUALS(testee.toString(true), "Minefield(77)");
    verif.verifyInteger("ID", MINEFIELD_NR);
    verif.verifyInteger("OWNER$", PLAYER_NR);
    verif.verifyString("OWNER", "Short Nine");
    verif.verifyString("OWNER.ADJ", "nine");

    // Inability to set
    TS_ASSERT_THROWS(verif.setIntegerValue("LOC.X", 1000), interpreter::Error);
    TS_ASSERT_THROWS(verif.setIntegerValue("OWNER$", 3), interpreter::Error);

    // Deletion
    g->currentTurn().universe().minefields().erase(MINEFIELD_NR);
    verif.verifyNull("ID");
    verif.verifyNull("OWNER");
}

/** Test iteration. */
void
TestGameInterfaceMinefieldContext::testIteration()
{
    // Environment
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::Game> g = *new game::Game();

    Minefield& mf = *g->currentTurn().universe().minefields().create(100);
    mf.addReport(game::map::Point(1200, 1300), 1, Minefield::IsWeb, Minefield::UnitsKnown, 400, 15, Minefield::MinefieldSwept);
    mf.internalCheck(15, root->hostVersion(), root->hostConfiguration());

    Minefield& mf2 = *g->currentTurn().universe().minefields().create(200);
    mf2.addReport(game::map::Point(2000, 4000), 2, Minefield::IsWeb, Minefield::UnitsKnown, 500, 15, Minefield::MinefieldSwept);
    mf2.internalCheck(15, root->hostVersion(), root->hostConfiguration());

    afl::string::NullTranslator tx;

    // Instance
    game::interface::MinefieldContext testee(100, root, g, tx);
    interpreter::test::ContextVerifier verif(testee, "testIteration");
    verif.verifyInteger("ID", 100);
    TS_ASSERT(testee.next());
    verif.verifyInteger("ID", 200);
    TS_ASSERT(!testee.next());
}

/** Test usage of commands. */
void
TestGameInterfaceMinefieldContext::testCommand()
{
    // Environment
    const int PLAYER_NR = 2;
    const int MINEFIELD_NR = 22;

    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::Game> g = *new game::Game();
    Minefield& mf = *g->currentTurn().universe().minefields().create(MINEFIELD_NR);
    mf.addReport(game::map::Point(1200, 1300), PLAYER_NR, Minefield::IsWeb, Minefield::UnitsKnown, 400, 15, Minefield::MinefieldSwept);
    mf.internalCheck(15, root->hostVersion(), root->hostConfiguration());

    afl::string::NullTranslator tx;

    // Instance
    game::interface::MinefieldContext testee(MINEFIELD_NR, root, g, tx);
    std::auto_ptr<afl::data::Value> meth(interpreter::test::ContextVerifier(testee, "testCommand").getValue("MARK"));

    // Invoke as command
    interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(meth.get());
    TS_ASSERT(cv != 0);
    interpreter::test::ValueVerifier(*cv, "testCommand").verifyBasics();
    {
        afl::sys::Log log;
        afl::io::NullFileSystem fs;
        interpreter::World world(log, tx, fs);
        afl::data::Segment seg;
        interpreter::Process proc(world, "dummy", 1);
        TS_ASSERT_THROWS_NOTHING(cv->call(proc, seg, false));
    }

    // Verify that command was executed
    TS_ASSERT(mf.isMarked());
}

/** Test factory function. */
void
TestGameInterfaceMinefieldContext::testCreate()
{
    const int PLAYER_NR = 2;
    const int MINEFIELD_NR = 22;

    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    Minefield& mf = *session.getGame()->currentTurn().universe().minefields().create(MINEFIELD_NR);
    mf.addReport(game::map::Point(1200, 1300), PLAYER_NR, Minefield::IsWeb, Minefield::UnitsKnown, 400, 15, Minefield::MinefieldSwept);
    mf.internalCheck(15, session.getRoot()->hostVersion(), session.getRoot()->hostConfiguration());

    // Success case
    {
        std::auto_ptr<game::interface::MinefieldContext> ctx(game::interface::MinefieldContext::create(MINEFIELD_NR, session, false));
        TS_ASSERT(ctx.get() != 0);
        TS_ASSERT_EQUALS(ctx->getObject(), &mf);
    }

    // Failure case
    {
        std::auto_ptr<game::interface::MinefieldContext> ctx(game::interface::MinefieldContext::create(MINEFIELD_NR+1, session, false));
        TS_ASSERT(ctx.get() == 0);
    }

    // Force
    {
        std::auto_ptr<game::interface::MinefieldContext> ctx(game::interface::MinefieldContext::create(MINEFIELD_NR+1, session, true));
        TS_ASSERT(ctx.get() != 0);
        TS_ASSERT(ctx->getObject() == 0);

        interpreter::test::ContextVerifier verif(*ctx, "create empty");
        verif.verifyNull("ID");
        verif.verifyNull("OWNER");
        TS_ASSERT_THROWS(verif.setIntegerValue("ID", 300), interpreter::Error);
        TS_ASSERT_THROWS(verif.setIntegerValue("OWNER$", 3), interpreter::Error);
    }
}

/** Test factory function on empty session.
    Even with force=true, this will not create an object. */
void
TestGameInterfaceMinefieldContext::testCreateEmpty()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;

    // No game
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        std::auto_ptr<game::interface::MinefieldContext> ctx(game::interface::MinefieldContext::create(1, session, true));
        TS_ASSERT(ctx.get() == 0);
    }

    // No root
    {
        game::Session session(tx, fs);
        session.setGame(new game::Game());
        std::auto_ptr<game::interface::MinefieldContext> ctx(game::interface::MinefieldContext::create(1, session, true));
        TS_ASSERT(ctx.get() == 0);
    }
}
