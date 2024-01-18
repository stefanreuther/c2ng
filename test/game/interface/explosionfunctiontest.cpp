/**
  *  \file test/game/interface/explosionfunctiontest.cpp
  *  \brief Test for game::interface::ExplosionFunction
  */

#include "game/interface/explosionfunction.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** General tests. */
AFL_TEST("game.interface.ExplosionFunction:basics", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    session.getGame()->currentTurn().universe().explosions().add(game::map::Explosion(1, game::map::Point(1000, 1020)));
    session.getGame()->currentTurn().universe().explosions().add(game::map::Explosion(0, game::map::Point(2000, 1020)));

    // Test basic properties
    game::interface::ExplosionFunction testee(session);
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

    a.checkEqual("01. getDimension", testee.getDimension(0), 0);

    // Cannot invoke directly
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("11. get"), testee.get(args), interpreter::Error);
    }

    // Test iteration
    {
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        a.checkNonNull("21. makeFirstContext", result.get());
        interpreter::test::ContextVerifier(*result, a("22. makeFirstContext")).verifyInteger("LOC.X", 1000);
    }

    // Test set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("31. set"), testee.set(args, 0), interpreter::Error);
    }
}

/** Test empty session. */
AFL_TEST("game.interface.ExplosionFunction:null", a)
{
    // Empty session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    game::interface::ExplosionFunction testee(session);
    interpreter::test::verifyNewNull(a, testee.makeFirstContext());
}
