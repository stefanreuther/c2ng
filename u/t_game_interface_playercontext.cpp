/**
  *  \file u/t_game_interface_playercontext.cpp
  *  \brief Test for game::interface::PlayerContext
  */

#include "game/interface/playercontext.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/test/root.hpp"
#include "interpreter/test/contextverifier.hpp"

/** Test basics: general behaviour, specific properties. */
void
TestGameInterfacePlayerContext::testBasics()
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
    interpreter::test::ContextVerifier verif(testee, "testBasics");
    verif.verifyBasics();
    verif.verifySerializable(interpreter::TagNode::Tag_Player, PLAYER_NR, afl::base::Nothing);
    verif.verifyTypes();
    TS_ASSERT(testee.getObject() == 0);

    // Specific properties
    TS_ASSERT_EQUALS(testee.toString(true), "Player(8)");
    verif.verifyInteger("RACE$", PLAYER_NR);
    verif.verifyString("RACE.SHORT", "eight");

    // Cannot modify
    TS_ASSERT_THROWS(verif.setIntegerValue("RACE$", 7), interpreter::Error);
}

/** Test iteration. */
void
TestGameInterfacePlayerContext::testIteration()
{
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::Game> g = *new game::Game();
    afl::string::NullTranslator tx;

    r->playerList().create(3);
    r->playerList().create(7);
    r->playerList().create(8);

    // Verify
    game::interface::PlayerContext testee(3, g, r, tx);
    interpreter::test::ContextVerifier verif(testee, "testBasics");
    verif.verifyInteger("RACE$", 3);
    TS_ASSERT(testee.next());
    verif.verifyInteger("RACE$", 7);
    TS_ASSERT(testee.next());
    verif.verifyInteger("RACE$", 8);
    TS_ASSERT(!testee.next());
}

/** Test creation using factory function. */
void
TestGameInterfacePlayerContext::testCreate()
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
        TS_ASSERT(p.get() != 0);
        interpreter::test::ContextVerifier(*p, "testCreate 3").verifyInteger("RACE$", 3);
    }

    // ...and for 0, which exists by default (but only publishes RACE$ for now)
    {
        std::auto_ptr<game::interface::PlayerContext> p(game::interface::PlayerContext::create(0, session));
        TS_ASSERT(p.get() != 0);
        interpreter::test::ContextVerifier(*p, "testCreate 0").verifyInteger("RACE$", 0);
        interpreter::test::ContextVerifier(*p, "testCreate 0").verifyNull("RACE");
    }

    // ...but not for any other Id.
    {
        std::auto_ptr<game::interface::PlayerContext> p(game::interface::PlayerContext::create(10, session));
        TS_ASSERT(p.get() == 0);
    }
}

/** Test creation using factory function on empty session. */
void
TestGameInterfacePlayerContext::testCreateEmpty()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;

    // No game
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        std::auto_ptr<game::interface::PlayerContext> p(game::interface::PlayerContext::create(0, session));
        TS_ASSERT(p.get() == 0);
    }

    // No root
    {
        game::Session session(tx, fs);
        session.setGame(new game::Game());
        std::auto_ptr<game::interface::PlayerContext> p(game::interface::PlayerContext::create(0, session));
        TS_ASSERT(p.get() == 0);
    }
}

