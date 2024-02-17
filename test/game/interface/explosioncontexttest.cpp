/**
  *  \file test/game/interface/explosioncontexttest.cpp
  *  \brief Test for game::interface::ExplosionContext
  */

#include "game/interface/explosioncontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "interpreter/test/contextverifier.hpp"

/** General tests. */
AFL_TEST("game.interface.ExplosionContext:basics", a)
{
    // Infrastructure
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Turn> turn(*new game::Turn());

    // Add an explosion
    game::map::Explosion expl(1, game::map::Point(1000, 1020));
    expl.setShipName("Excalibur");
    expl.setShipId(23);
    turn->universe().explosions().add(expl);

    // Test it
    game::interface::ExplosionContext testee(1, turn, tx);
    interpreter::test::ContextVerifier v(testee, a);
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
AFL_TEST("game.interface.ExplosionContext:iteration", a)
{
    // Infrastructure
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Turn> turn(*new game::Turn());

    turn->universe().explosions().add(game::map::Explosion(1, game::map::Point(1000, 1020)));
    turn->universe().explosions().add(game::map::Explosion(0, game::map::Point(2000, 1020)));

    // Test it
    game::interface::ExplosionContext testee(1, turn, tx);
    interpreter::test::ContextVerifier v(testee, a);
    v.verifyInteger("LOC.X", 1000);
    a.check("01. next", testee.next());
    v.verifyInteger("LOC.X", 2000);
    a.check("02. next", !testee.next());
}

/** Test behaviour on non-existant object.
    Normally, such an ExplosionContext instance cannot be created. */
AFL_TEST("game.interface.ExplosionContext:null", a)
{
    // Infrastructure
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Turn> turn(*new game::Turn());

    // Test it
    game::interface::ExplosionContext testee(1, turn, tx);
    interpreter::test::ContextVerifier v(testee, a);
    v.verifyNull("LOC.X");
    v.verifyNull("NAME");
}

/** Test creation using factory function. */
AFL_TEST("game.interface.ExplosionContext:create", a)
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
        std::auto_ptr<game::interface::ExplosionContext> p(game::interface::ExplosionContext::create(1, session, session.getGame()->viewpointTurn()));
        a.checkNonNull("01. create", p.get());
        interpreter::test::ContextVerifier(*p, a("02. create")).verifyInteger("LOC.X", 1000);
    }

    // ...but not for any other Id.
    {
        std::auto_ptr<game::interface::ExplosionContext> p(game::interface::ExplosionContext::create(0, session, session.getGame()->viewpointTurn()));
        a.checkNull("11. create 0", p.get());
    }
    {
        std::auto_ptr<game::interface::ExplosionContext> p(game::interface::ExplosionContext::create(10, session, session.getGame()->viewpointTurn()));
        a.checkNull("12. create 10", p.get());
    }
}

/** Test (inability to) set property values. */
AFL_TEST("game.interface.ExplosionContext:set", a)
{
    // Infrastructure
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Turn> turn(*new game::Turn());

    turn->universe().explosions().add(game::map::Explosion(1, game::map::Point(1000, 1020)));

    // Test it
    game::interface::ExplosionContext testee(1, turn, tx);
    interpreter::test::ContextVerifier v(testee, a);
    AFL_CHECK_THROWS(a, v.setIntegerValue("LOC.X", 2000), interpreter::Error);
}
