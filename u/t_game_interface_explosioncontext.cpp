/**
  *  \file u/t_game_interface_explosioncontext.cpp
  *  \brief Test for game::interface::ExplosionContext
  */

#include "game/interface/explosioncontext.hpp"

#include "t_game_interface.hpp"
#include "game/session.hpp"
#include "game/game.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/nullfilesystem.hpp"

/** General tests. */
void
TestGameInterfaceExplosionContext::testIt()
{
    // Infrastructure
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());

    // Add an explosion
    game::map::Explosion expl(1, game::map::Point(1000, 1020));
    expl.setShipName("Excalibur");
    expl.setShipId(23);
    session.getGame()->currentTurn().universe().explosions().add(expl);

    // Test it
    game::interface::ExplosionContext testee(1, session, session.getGame()->currentTurn());
    interpreter::test::ContextVerifier v(testee, "testIt");
    v.verifyTypes();
    v.verifyBasics();
    v.verifyNotSerializable();

    // Verify some values
    v.verifyInteger("ID", 1);
    v.verifyInteger("ID.SHIP", 23);
    v.verifyInteger("LOC.X", 1000);
    v.verifyInteger("LOC.Y", 1020);
    v.verifyString("TYPE", "Explosion");
    v.verifyString("TYPE.SHORT", "E");
    v.verifyString("NAME.SHIP", "Excalibur");
    v.verifyString("NAME", "Explosion of Excalibur (#23)");
}

/** Test iteration. */
void
TestGameInterfaceExplosionContext::testIteration()
{
    // Infrastructure
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());

    session.getGame()->currentTurn().universe().explosions().add(game::map::Explosion(1, game::map::Point(1000, 1020)));
    session.getGame()->currentTurn().universe().explosions().add(game::map::Explosion(0, game::map::Point(2000, 1020)));

    // Test it
    game::interface::ExplosionContext testee(1, session, session.getGame()->currentTurn());
    interpreter::test::ContextVerifier v(testee, "testIt");
    v.verifyInteger("LOC.X", 1000);
    TS_ASSERT(testee.next());
    v.verifyInteger("LOC.X", 2000);
    TS_ASSERT(!testee.next());
}

/** Test behaviour on non-existant object.
    Normally, such an ExplosionContext instance cannot be created. */
void
TestGameInterfaceExplosionContext::testNull()
{
    // Infrastructure
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());

    // Test it
    game::interface::ExplosionContext testee(1, session, session.getGame()->currentTurn());
    interpreter::test::ContextVerifier v(testee, "testIt");
    v.verifyNull("LOC.X");
    v.verifyNull("NAME");
}

/** Test creation using factory function. */
void
TestGameInterfaceExplosionContext::testCreate()
{
    // Infrastructure
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());

    // Add an explosion
    game::map::Explosion expl(42, game::map::Point(1000, 1020));
    expl.setShipName("Excalibur");
    expl.setShipId(23);
    session.getGame()->currentTurn().universe().explosions().add(expl);

    // Can create an ExplosionContext for ID 1
    {
        std::auto_ptr<game::interface::ExplosionContext> p(game::interface::ExplosionContext::create(1, session));
        TS_ASSERT(p.get() != 0);
        interpreter::test::ContextVerifier(*p, "testCreate").verifyInteger("LOC.X", 1000);
    }

    // ...but not for any other Id.
    {
        std::auto_ptr<game::interface::ExplosionContext> p(game::interface::ExplosionContext::create(0, session));
        TS_ASSERT(p.get() == 0);
    }
    {
        std::auto_ptr<game::interface::ExplosionContext> p(game::interface::ExplosionContext::create(10, session));
        TS_ASSERT(p.get() == 0);
    }
}

/** Test creation using factory function, empty session. */
void
TestGameInterfaceExplosionContext::testCreateEmpty()
{
    // Infrastructure
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Cannot create an ExplosionContext
    std::auto_ptr<game::interface::ExplosionContext> p(game::interface::ExplosionContext::create(1, session));
    TS_ASSERT(p.get() == 0);
}

/** Test (inability to) set property values. */
void
TestGameInterfaceExplosionContext::testSet()
{
    // Infrastructure
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());

    session.getGame()->currentTurn().universe().explosions().add(game::map::Explosion(1, game::map::Point(1000, 1020)));

    // Test it
    game::interface::ExplosionContext testee(1, session, session.getGame()->currentTurn());
    interpreter::test::ContextVerifier v(testee, "testIt");
    TS_ASSERT_THROWS(v.setIntegerValue("LOC.X", 2000), interpreter::Error);
}

