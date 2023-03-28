/**
  *  \file u/t_game_interface_vcrcontext.cpp
  *  \brief Test for game::interface::VcrContext
  */

#include "game/interface/vcrcontext.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "game/vcr/test/battle.hpp"
#include "game/vcr/test/database.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/test/contextverifier.hpp"

using game::vcr::test::Battle;

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

    Battle& addBattle(game::Session& session)
    {
        afl::base::Ptr<game::vcr::test::Database> db = new game::vcr::test::Database();
        Battle& b = db->addBattle();
        b.addObject(makeShip(10, 5), 0);
        b.addObject(makeShip(20, 6), 7);
        b.addObject(makeShip(30, 7), 7);
        session.getGame()->currentTurn().setBattles(db);
        return b;
    }

    void addMultipleBattles(game::Session& session)
    {
        afl::base::Ptr<game::vcr::test::Database> db = new game::vcr::test::Database();
        db->addBattle().addObject(makeShip(10, 5), 0);
        db->addBattle().addObject(makeShip(20, 6), 0);
        db->addBattle().addObject(makeShip(30, 7), 0);
        session.getGame()->currentTurn().setBattles(db);
    }
}

/** General tests. */
void
TestGameInterfaceVcrContext::testIt()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.setGame(new game::Game());
    Battle& b = addBattle(session);
    b.setAuxiliaryInformation(Battle::aiFlags, 4444);

    // Instance
    game::interface::VcrContext testee(0, session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList());
    interpreter::test::ContextVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyTypes();
    verif.verifyNotSerializable();
    TS_ASSERT(testee.getObject() == 0);

    // Verify some properties
    verif.verifyInteger("CAPABILITIES", 4444);
    verif.verifyInteger("LEFT.ID",  10);
    verif.verifyInteger("RIGHT.ID", 20);
}

/** Test iteration. */
void
TestGameInterfaceVcrContext::testIteration()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.setGame(new game::Game());
    addMultipleBattles(session);

    // Verify
    game::interface::VcrContext testee(0, session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList());
    interpreter::test::ContextVerifier verif(testee, "testIt");
    verif.verifyInteger("LEFT.ID",  10);
    TS_ASSERT(testee.next());
    verif.verifyInteger("LEFT.ID",  20);
    TS_ASSERT(testee.next());
    verif.verifyInteger("LEFT.ID",  30);
    TS_ASSERT(!testee.next());
}

/** Test factory function. */
void
TestGameInterfaceVcrContext::testCreate()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.setGame(new game::Game());
    addMultipleBattles(session);

    // In range
    {
        std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(1, session));
        TS_ASSERT(ctx.get() != 0);
        interpreter::test::ContextVerifier(*ctx, "(1)").verifyInteger("LEFT.ID", 20);
    }

    // Out of range
    {
        std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(3, session));
        TS_ASSERT(ctx.get() == 0);
    }
}

/** Test factory function on empty session. */
void
TestGameInterfaceVcrContext::testCreateEmpty()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;

    // No root
    {
        game::Session session(tx, fs);
        session.setShipList(new game::spec::ShipList());
        session.setGame(new game::Game());
        addMultipleBattles(session);

        std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(0, session));
        TS_ASSERT(ctx.get() == 0);
    }

    // No ship list
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setGame(new game::Game());
        addMultipleBattles(session);

        std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(0, session));
        TS_ASSERT(ctx.get() == 0);
    }

    // No game
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setShipList(new game::spec::ShipList());

        std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(0, session));
        TS_ASSERT(ctx.get() == 0);
    }

    // No battles
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setShipList(new game::spec::ShipList());
        session.setGame(new game::Game());

        std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(0, session));
        TS_ASSERT(ctx.get() == 0);
    }
}

