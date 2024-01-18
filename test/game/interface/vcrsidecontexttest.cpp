/**
  *  \file test/game/interface/vcrsidecontexttest.cpp
  *  \brief Test for game::interface::VcrSideContext
  */

#include "game/interface/vcrsidecontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/vcr/object.hpp"
#include "game/vcr/test/battle.hpp"
#include "game/vcr/test/database.hpp"
#include "interpreter/test/contextverifier.hpp"

namespace {
    game::vcr::Object makeShip(game::Id_t id, int owner)
    {
        game::vcr::Object o;
        o.setId(id);
        o.setOwner(owner);
        o.setIsPlanet(false);
        o.setName("X");
        return o;
    }

    void addDefaultBattle(game::Session& session)
    {
        afl::base::Ptr<game::vcr::test::Database> db = new game::vcr::test::Database();
        game::vcr::test::Battle& b = db->addBattle();
        b.addObject(makeShip(10, 5), 0);
        b.addObject(makeShip(20, 6), 7);
        b.addObject(makeShip(30, 7), 7);
        session.getGame()->currentTurn().setBattles(db);
    }
}

/** Test general operation. */
AFL_TEST("game.interface.VcrSideContext:basics", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    addDefaultBattle(session);

    // Test
    game::interface::VcrSideContext testee(0, 1, session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList());
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyTypes();
    verif.verifyNotSerializable();
    a.checkNull("01. getObject", testee.getObject());

    // Verify some properties
    verif.verifyInteger("OWNER$", 6);
    verif.verifyInteger("ID", 20);
    verif.verifyInteger("INDEX", 2);
    a.check("11. next", testee.next());
    verif.verifyInteger("OWNER$", 7);
    verif.verifyInteger("ID", 30);
    verif.verifyInteger("INDEX", 3);
    a.check("12. next", !testee.next());
}

/** Test behaviour on null battle.
    Can normally not happen. */
AFL_TEST("game.interface.VcrSideContext:null", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());

    // Test
    game::interface::VcrSideContext testee(20, 17, session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList());
    interpreter::test::ContextVerifier verif(testee, a);

    // Verify some properties
    verif.verifyNull("OWNER$");
    verif.verifyNull("ID");
    verif.verifyInteger("INDEX", 18);
}

/** Test creation using factory function. */
AFL_TEST("game.interface.VcrSideContext:create", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    addDefaultBattle(session);

    // Success case
    {
        std::auto_ptr<game::interface::VcrSideContext> p(game::interface::VcrSideContext::create(0, 2, session));
        a.checkNonNull("01. get", p.get());
        interpreter::test::ContextVerifier(*p, a("02. get")).verifyInteger("INDEX", 3);
    }

    // Error, battle number out of range
    {
        std::auto_ptr<game::interface::VcrSideContext> p(game::interface::VcrSideContext::create(1, 0, session));
        a.checkNull("11. index range error", p.get());
    }

    // Error, side out of range
    {
        std::auto_ptr<game::interface::VcrSideContext> p(game::interface::VcrSideContext::create(0, 3, session));
        a.checkNull("21. side range error", p.get());
    }
}

// No ship list
AFL_TEST("game.interface.VcrSideContext:error:no-shiplist", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    addDefaultBattle(session);

    std::auto_ptr<game::interface::VcrSideContext> p(game::interface::VcrSideContext::create(0, 0, session));
    a.checkNull("ctx", p.get());
}

// No root
AFL_TEST("game.interface.VcrSideContext:error:no-root", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.setGame(new game::Game());
    addDefaultBattle(session);

    std::auto_ptr<game::interface::VcrSideContext> p(game::interface::VcrSideContext::create(0, 0, session));
    a.checkNull("ctx", p.get());
}

// No game
AFL_TEST("game.interface.VcrSideContext:error:no-game", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());

    std::auto_ptr<game::interface::VcrSideContext> p(game::interface::VcrSideContext::create(0, 0, session));
    a.checkNull("ctx", p.get());
}

// No battles
AFL_TEST("game.interface.VcrSideContext:error:no-battles", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());

    std::auto_ptr<game::interface::VcrSideContext> p(game::interface::VcrSideContext::create(0, 0, session));
    a.checkNull("ctx", p.get());
}
