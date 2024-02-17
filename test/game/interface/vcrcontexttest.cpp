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

using afl::base::Ptr;
using afl::base::Ref;
using game::HostVersion;
using game::Root;
using game::spec::ShipList;
using game::vcr::test::Battle;
using game::vcr::test::Database;

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

    Battle& addBattle(Database& db)
    {
        Battle& b = db.addBattle();
        b.addObject(makeShip(10, 5), 0);
        b.addObject(makeShip(20, 6), 7);
        b.addObject(makeShip(30, 7), 7);
        return b;
    }

    void addMultipleBattles(game::Session& session)
    {
        Ptr<Database> db = new Database();
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
    Ref<Root> r(game::test::makeRoot(HostVersion()));
    Ref<ShipList> sl(*new ShipList());
    Ptr<Database> db(new Database());
    Battle& b = addBattle(*db);
    b.setAuxiliaryInformation(Battle::aiFlags, 4444);

    // Instance
    game::interface::VcrContext testee(0, tx, r, db, sl);
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
    session.setRoot(game::test::makeRoot(HostVersion()).asPtr());
    session.setShipList(new ShipList());
    session.setGame(new game::Game());
    addMultipleBattles(session);

    // Verify
    game::interface::VcrContext testee(0, tx, *session.getRoot(), session.getGame()->currentTurn().getBattles(), *session.getShipList());
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
    session.setRoot(game::test::makeRoot(HostVersion()).asPtr());
    session.setShipList(new ShipList());
    session.setGame(new game::Game());
    addMultipleBattles(session);

    // In range
    {
        std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(1, session, session.getGame()->currentTurn().getBattles()));
        a.checkNonNull("01. get", ctx.get());
        interpreter::test::ContextVerifier(*ctx, a("01. get")).verifyInteger("LEFT.ID", 20);
    }

    // Out of range
    {
        std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(3, session, session.getGame()->currentTurn().getBattles()));
        a.checkNull("11. out of range", ctx.get());
    }
}

// No root
AFL_TEST("game.interface.VcrContext:error:no-root", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new ShipList());
    session.setGame(new game::Game());
    addMultipleBattles(session);

    std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(0, session, session.getGame()->currentTurn().getBattles()));
    a.checkNull("ctx", ctx.get());
}

// No ship list
AFL_TEST("game.interface.VcrContext:error:no-shiplist", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(HostVersion()).asPtr());
    session.setGame(new game::Game());
    addMultipleBattles(session);

    std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(0, session, session.getGame()->currentTurn().getBattles()));
    a.checkNull("ctx", ctx.get());
}

// No battles
AFL_TEST("game.interface.VcrContext:error:no-battles", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(HostVersion()).asPtr());
    session.setShipList(new ShipList());
    session.setGame(new game::Game());

    std::auto_ptr<game::interface::VcrContext> ctx(game::interface::VcrContext::create(0, session, session.getGame()->currentTurn().getBattles()));
    a.checkNull("ctx", ctx.get());
}
