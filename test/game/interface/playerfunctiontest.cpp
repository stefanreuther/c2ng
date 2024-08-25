/**
  *  \file test/game/interface/playerfunctiontest.cpp
  *  \brief Test for game::interface::PlayerFunction
  */

#include "game/interface/playerfunction.hpp"

#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/player.hpp"
#include "game/playerlist.hpp"
#include "game/test/root.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** General tests. */
AFL_TEST("game.interface.PlayerFunction:basics", a)
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
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

    a.checkEqual("01. getDimension 0", testee.getDimension(0), 1U);
    a.checkEqual("02. getDimension 1", testee.getDimension(1), 6U);

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNonNull("11. get(3)", result.get());
        interpreter::test::ContextVerifier(*result, a("12. get(3)")).verifyInteger("RACE$", 3);
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
        // range error
        afl::data::Segment seg;
        seg.pushBackInteger(6);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNull("23. range error", result.get());
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
        interpreter::test::ContextVerifier(*result, a("42. makeFirstContext")).verifyInteger("RACE$", 3);
    }

    // Test set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("51. set"), testee.set(args, 0), interpreter::Error);
    }
}

// Empty session
AFL_TEST("game.interface.PlayerFunction:empty:no-root", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    game::interface::PlayerFunction testee(session);
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    a.checkNull("makeFirstContext", result.get());
}

// Session populated with empty objects
AFL_TEST("game.interface.PlayerFunction:empty:no-players", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());

    game::interface::PlayerFunction testee(session);
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    a.checkNull("makeFirstContext", result.get());
}
