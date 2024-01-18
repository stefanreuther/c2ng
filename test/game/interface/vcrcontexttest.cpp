/**
  *  \file test/game/interface/vcrcontexttest.cpp
  *  \brief Test for game::interface::VcrContext
  */

#include "game/interface/vcrcontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "game/vcr/test/battle.hpp"
#include "game/vcr/test/database.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

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
AFL_TEST("game.interface.VcrContext:basics", a)
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
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyTypes();
    verif.verifyNotSerializable();
    a.checkNull("getObject", testee.getObject());

    // Verify some properties
    verif.verifyInteger("CAPABILITIES", 4444);
    verif.verifyInteger("LEFT.ID",  10);
    verif.verifyInteger("RIGHT.ID", 20);
}

/** Test iteration. */
AFL_TEST("game.interface.VcrContext:iteration", a)
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
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyInteger("LEFT.ID",  10);
    a.check("01. next", testee.next());
    verif.verifyInteger("LEFT.ID",  20);
    a.check("02. next", testee.next());
    verif.verifyInteger("LEFT.ID",  30);
    a.check("03. next", !testee.next());
}

/** Test factory function. */
AFL_TEST("game.interface.VcrContext:create", a)
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
        a.checkNonNull("01. get", ctx.get());
        interpreter::test::ContextVerifier(*ctx, a("01. get")).verifyInteger("LEFT.ID", 20);
    }

    // Out of range
    {
        std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(3, session));
        a.checkNull("11. out of range", ctx.get());
    }
}

// No root
AFL_TEST("game.interface.VcrContext:error:no-root", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.setGame(new game::Game());
    addMultipleBattles(session);

    std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(0, session));
    a.checkNull("ctx", ctx.get());
}

// No ship list
AFL_TEST("game.interface.VcrContext:error:no-shiplist", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    addMultipleBattles(session);

    std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(0, session));
    a.checkNull("ctx", ctx.get());
}

// No game
AFL_TEST("game.interface.VcrContext:error:no-game", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());

    std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(0, session));
    a.checkNull("ctx", ctx.get());
}

// No battles
AFL_TEST("game.interface.VcrContext:error:no-battles", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.setGame(new game::Game());

    std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(0, session));
    a.checkNull("ctx", ctx.get());
}
