/**
  *  \file test/game/interface/ufofunctiontest.cpp
  *  \brief Test for game::interface::UfoFunction
  */

#include "game/interface/ufofunction.hpp"

#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** General tests. */
AFL_TEST("game.interface.UfoFunction:basics", a)
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
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();
    a.checkEqual("01. getDimension 0", testee.getDimension(0), 1U);
    a.check("02. getDimension 1", testee.getDimension(1) >= 10000);

    // Successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(77);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> ctx(testee.get(args));
        a.checkNonNull("11. get", ctx.get());
        interpreter::test::ContextVerifier(*ctx, a("12. get")).verifyInteger("ID", 77);
    }

    // Invoke with unknown Id
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> ctx(testee.get(args));
        a.checkNull("21. get unknown", ctx.get());
    }

    // Invoke with null
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> ctx(testee.get(args));
        a.checkNull("31. null", ctx.get());
    }

    // Arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("41. arity error"), testee.get(args), interpreter::Error);
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackString("77");
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("51. type error"), testee.get(args), interpreter::Error);
    }

    // Set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(77);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("61. set"), testee.set(args, 0), interpreter::Error);
    }

    // Iteration
    {
        std::auto_ptr<interpreter::Context> ctx(testee.makeFirstContext());
        a.checkNonNull("71. makeFirstContext", ctx.get());
        interpreter::test::ContextVerifier(*ctx, a("72. makeFirstContext")).verifyInteger("ID", 51);
    }
}

// No game
AFL_TEST("game.interface.UfoFunction:empty-session", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    game::interface::UfoFunction testee(session);

    afl::data::Segment seg;
    seg.pushBackInteger(2);
    interpreter::Arguments args(seg, 0, 1);
    std::auto_ptr<interpreter::Context> ctx(testee.get(args));
    a.checkNull("01. get", ctx.get());

    ctx.reset(testee.makeFirstContext());
    a.checkNull("11. makeFirstContext", ctx.get());
}

// No objects
AFL_TEST("game.interface.UfoFunction:empty-universe", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    game::interface::UfoFunction testee(session);

    afl::data::Segment seg;
    seg.pushBackInteger(2);
    interpreter::Arguments args(seg, 0, 1);
    std::auto_ptr<interpreter::Context> ctx(testee.get(args));
    a.checkNull("01. get", ctx.get());

    ctx.reset(testee.makeFirstContext());
    a.checkNull("11. makeFirstContext", ctx.get());
}
