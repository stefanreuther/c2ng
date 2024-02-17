/**
  *  \file test/game/interface/minefieldcontexttest.cpp
  *  \brief Test for game::interface::MinefieldContext
  */

#include "game/interface/minefieldcontext.hpp"

#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/minefield.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/world.hpp"

using game::Player;
using game::map::Minefield;

/** Test basics: general behaviour, specific properties. */
AFL_TEST("game.interface.MinefieldContext:basics", a)
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
    game::interface::MinefieldContext testee(MINEFIELD_NR, root, g, g->currentTurn(), tx);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifySerializable(interpreter::TagNode::Tag_Minefield, MINEFIELD_NR, afl::base::Nothing);
    verif.verifyTypes();
    a.checkEqual("01. getObject", testee.getObject(), &mf);

    // Specific properties
    a.checkEqual("11. toString", testee.toString(true), "Minefield(77)");
    verif.verifyInteger("ID", MINEFIELD_NR);
    verif.verifyInteger("OWNER$", PLAYER_NR);
    verif.verifyString("OWNER", "Short Nine");
    verif.verifyString("OWNER.ADJ", "nine");

    // Inability to set
    AFL_CHECK_THROWS(a("21. set LOC.X"), verif.setIntegerValue("LOC.X", 1000), interpreter::Error);
    AFL_CHECK_THROWS(a("22. set OWNER$"), verif.setIntegerValue("OWNER$", 3), interpreter::Error);

    // Deletion
    g->currentTurn().universe().minefields().erase(MINEFIELD_NR);
    verif.verifyNull("ID");
    verif.verifyNull("OWNER");
}

/** Test iteration. */
AFL_TEST("game.interface.MinefieldContext:iteration", a)
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
    game::interface::MinefieldContext testee(100, root, g, g->currentTurn(), tx);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyInteger("ID", 100);
    a.check("01. next", testee.next());
    verif.verifyInteger("ID", 200);
    a.check("02. next", !testee.next());
}

/** Test usage of commands. */
AFL_TEST("game.interface.MinefieldContext:commands", a)
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
    game::interface::MinefieldContext testee(MINEFIELD_NR, root, g, g->currentTurn(), tx);
    std::auto_ptr<afl::data::Value> meth(interpreter::test::ContextVerifier(testee, a).getValue("MARK"));

    // Invoke as command
    interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(meth.get());
    a.checkNonNull("01. cv", cv);
    interpreter::test::ValueVerifier(*cv, a).verifyBasics();
    {
        afl::sys::Log log;
        afl::io::NullFileSystem fs;
        interpreter::World world(log, tx, fs);
        afl::data::Segment seg;
        interpreter::Process proc(world, "dummy", 1);
        AFL_CHECK_SUCCEEDS(a("02. call"), cv->call(proc, seg, false));
    }

    // Verify that command was executed
    a.check("11. isMarked", mf.isMarked());
}

/** Test factory function. */
AFL_TEST("game.interface.MinefieldContext:create", a)
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
        std::auto_ptr<game::interface::MinefieldContext> ctx(game::interface::MinefieldContext::create(MINEFIELD_NR, session, *session.getGame(), session.getGame()->currentTurn(), false));
        a.checkNonNull("01. create", ctx.get());
        a.checkEqual("02. getObject", ctx->getObject(), &mf);
    }

    // Failure case
    {
        std::auto_ptr<game::interface::MinefieldContext> ctx(game::interface::MinefieldContext::create(MINEFIELD_NR+1, session, *session.getGame(), session.getGame()->currentTurn(), false));
        a.checkNull("11. wrong id", ctx.get());
    }

    // Force
    {
        std::auto_ptr<game::interface::MinefieldContext> ctx(game::interface::MinefieldContext::create(MINEFIELD_NR+1, session, *session.getGame(), session.getGame()->currentTurn(), true));
        a.checkNonNull("21. create", ctx.get());
        a.checkNull("22. getObject", ctx->getObject());

        interpreter::test::ContextVerifier verif(*ctx, a("create empty"));
        verif.verifyNull("ID");
        verif.verifyNull("OWNER");
        AFL_CHECK_THROWS(a("31. set ID"), verif.setIntegerValue("ID", 300), interpreter::Error);
        AFL_CHECK_THROWS(a("32. set OWNER$"), verif.setIntegerValue("OWNER$", 3), interpreter::Error);
    }
}

/** Test factory function on empty session.
    Even with force=true, this will not create an object. */

// No root
AFL_TEST("game.interface.MinefieldContext:create:no-root", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    std::auto_ptr<game::interface::MinefieldContext> ctx(game::interface::MinefieldContext::create(1, session, *session.getGame(), session.getGame()->currentTurn(), true));
    a.checkNull("", ctx.get());
}
