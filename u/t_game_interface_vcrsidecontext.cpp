/**
  *  \file u/t_game_interface_vcrsidecontext.cpp
  *  \brief Test for game::interface::VcrSideContext
  */

#include "game/interface/vcrsidecontext.hpp"

#include "t_game_interface.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/game.hpp"
#include "game/vcr/test/database.hpp"
#include "game/vcr/test/battle.hpp"
#include "game/vcr/object.hpp"
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
void
TestGameInterfaceVcrSideContext::testIt()
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
    interpreter::test::ContextVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyTypes();
    verif.verifyNotSerializable();
    TS_ASSERT(testee.getObject() == 0);

    // Verify some properties
    verif.verifyInteger("OWNER$", 6);
    verif.verifyInteger("ID", 20);
    verif.verifyInteger("INDEX", 2);
    TS_ASSERT(testee.next());
    verif.verifyInteger("OWNER$", 7);
    verif.verifyInteger("ID", 30);
    verif.verifyInteger("INDEX", 3);
    TS_ASSERT(!testee.next());
}

/** Test behaviour on null battle.
    Can normally not happen. */
void
TestGameInterfaceVcrSideContext::testNull()
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
    interpreter::test::ContextVerifier verif(testee, "testNull");

    // Verify some properties
    verif.verifyNull("OWNER$");
    verif.verifyNull("ID");
    verif.verifyInteger("INDEX", 18);
}

/** Test creation using factory function. */
void
TestGameInterfaceVcrSideContext::testCreate()
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
        TS_ASSERT(p.get() != 0);
        interpreter::test::ContextVerifier(*p, "testCreate").verifyInteger("INDEX", 3);
    }

    // Error, battle number out of range
    {
        std::auto_ptr<game::interface::VcrSideContext> p(game::interface::VcrSideContext::create(1, 0, session));
        TS_ASSERT(p.get() == 0);
    }

    // Error, side out of range
    {
        std::auto_ptr<game::interface::VcrSideContext> p(game::interface::VcrSideContext::create(0, 3, session));
        TS_ASSERT(p.get() == 0);
    }
}

/** Test creation, missing preconditions. */
void
TestGameInterfaceVcrSideContext::testCreateEmpty()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;

    // No ship list
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setGame(new game::Game());
        addDefaultBattle(session);

        std::auto_ptr<game::interface::VcrSideContext> p(game::interface::VcrSideContext::create(0, 0, session));
        TS_ASSERT(p.get() == 0);
    }

    // No root
    {
        game::Session session(tx, fs);
        session.setShipList(new game::spec::ShipList());
        session.setGame(new game::Game());
        addDefaultBattle(session);

        std::auto_ptr<game::interface::VcrSideContext> p(game::interface::VcrSideContext::create(0, 0, session));
        TS_ASSERT(p.get() == 0);
    }

    // No game
    {
        game::Session session(tx, fs);
        session.setShipList(new game::spec::ShipList());
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());

        std::auto_ptr<game::interface::VcrSideContext> p(game::interface::VcrSideContext::create(0, 0, session));
        TS_ASSERT(p.get() == 0);
    }

    // No battles
    {
        game::Session session(tx, fs);
        session.setShipList(new game::spec::ShipList());
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setGame(new game::Game());

        std::auto_ptr<game::interface::VcrSideContext> p(game::interface::VcrSideContext::create(0, 0, session));
        TS_ASSERT(p.get() == 0);
    }
}

