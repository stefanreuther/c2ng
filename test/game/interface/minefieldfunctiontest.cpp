/**
  *  \file test/game/interface/minefieldfunctiontest.cpp
  *  \brief Test for game::interface::MinefieldFunction
  */

#include "game/interface/minefieldfunction.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.interface.MinefieldFunction:basics", a)
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
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

    a.checkEqual("01. getDimension 0", testee.getDimension(0), 1);
    a.checkEqual("02. getDimension 1", testee.getDimension(1), 201);

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(200);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNonNull("11. get", result.get());
        interpreter::test::ContextVerifier(*result, a("12. get")).verifyInteger("ID", 200);
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
        // non-existant - does not throw in c2ng
        afl::data::Segment seg;
        seg.pushBackInteger(22222);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNull("23. nonexistant", result.get());
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
        interpreter::test::ContextVerifier(*result, a("42. makeFirstContext")).verifyInteger("ID", 100);
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
// Empty session
AFL_TEST("game.interface.MinefieldFunction:makeFirstContext:no-game", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    game::interface::MinefieldFunction testee(session);
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    a.checkNull("get", result.get());

    // No dimension because no game
    a.checkEqual("getDimension", testee.getDimension(1), 0);
}

// Session populated with empty objects
AFL_TEST("game.interface.MinefieldFunction:makeFirstContext:no-objects", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());

    game::interface::MinefieldFunction testee(session);
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    a.checkNull("get", result.get());

    // Slot 0 is present (but empty)
    a.checkEqual("getDimension", testee.getDimension(1), 1);
}
