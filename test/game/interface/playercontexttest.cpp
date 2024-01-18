/**
  *  \file test/game/interface/playercontexttest.cpp
  *  \brief Test for game::interface::PlayerContext
  */

#include "game/interface/playercontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/test/root.hpp"
#include "interpreter/test/contextverifier.hpp"

/** Test basics: general behaviour, specific properties. */
AFL_TEST("game.interface.PlayerContext:basics", a)
{
    // Environment
    const int PLAYER_NR = 8;
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::Game> g = *new game::Game();
    afl::string::NullTranslator tx;

    game::Player& pl = *r->playerList().create(PLAYER_NR);
    pl.setName(game::Player::ShortName, "eight");

    // Instance
    game::interface::PlayerContext testee(PLAYER_NR, g, r, tx);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifySerializable(interpreter::TagNode::Tag_Player, PLAYER_NR, afl::base::Nothing);
    verif.verifyTypes();
    a.checkNull("01. getObject", testee.getObject());

    // Specific properties
    a.checkEqual("11. toString", testee.toString(true), "Player(8)");
    verif.verifyInteger("RACE$", PLAYER_NR);
    verif.verifyString("RACE.SHORT", "eight");

    // Cannot modify
    AFL_CHECK_THROWS(a("21. set RACE$"), verif.setIntegerValue("RACE$", 7), interpreter::Error);
}

/** Test iteration. */
AFL_TEST("game.interface.PlayerContext:iteration", a)
{
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::Game> g = *new game::Game();
    afl::string::NullTranslator tx;

    r->playerList().create(3);
    r->playerList().create(7);
    r->playerList().create(8);

    // Verify
    game::interface::PlayerContext testee(3, g, r, tx);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyInteger("RACE$", 3);
    a.check("01. next", testee.next());
    verif.verifyInteger("RACE$", 7);
    a.check("02. next", testee.next());
    verif.verifyInteger("RACE$", 8);
    a.check("03. next", !testee.next());
}

/** Test creation using factory function. */
AFL_TEST("game.interface.PlayerContext:create", a)
{
    // Given an environment with one player...
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());

    session.getRoot()->playerList().create(3);

    // ...I expect to be able to create a PlayerContext for it.
    {
        std::auto_ptr<game::interface::PlayerContext> p(game::interface::PlayerContext::create(3, session));
        a.checkNonNull("01. get 3", p.get());
        interpreter::test::ContextVerifier(*p, a("02. get 3")).verifyInteger("RACE$", 3);
    }

    // ...and for 0, which exists by default (but only publishes RACE$ for now)
    {
        std::auto_ptr<game::interface::PlayerContext> p(game::interface::PlayerContext::create(0, session));
        a.checkNonNull("11. get 0", p.get());
        interpreter::test::ContextVerifier(*p, a("12. get 0")).verifyInteger("RACE$", 0);
        interpreter::test::ContextVerifier(*p, a("13. get 0")).verifyNull("RACE");
    }

    // ...but not for any other Id.
    {
        std::auto_ptr<game::interface::PlayerContext> p(game::interface::PlayerContext::create(10, session));
        a.checkNull("21. get 10", p.get());
    }
}

// No game
AFL_TEST("game.interface.PlayerContext:create:no-game", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    std::auto_ptr<game::interface::PlayerContext> p(game::interface::PlayerContext::create(0, session));
    a.checkNull("get", p.get());
}

// No root
AFL_TEST("game.interface.PlayerContext:create:no-root", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    std::auto_ptr<game::interface::PlayerContext> p(game::interface::PlayerContext::create(0, session));
    a.checkNull("get", p.get());
}
