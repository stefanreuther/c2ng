/**
  *  \file u/t_game_interface_vcrsidefunction.cpp
  *  \brief Test for game::interface::VcrSideFunction
  */

#include "game/interface/vcrsidefunction.hpp"

#include "t_game_interface.hpp"
#include "game/session.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/game.hpp"
#include "game/vcr/test/database.hpp"
#include "game/vcr/test/battle.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/arguments.hpp"
#include "afl/data/segment.hpp"
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

void
TestGameInterfaceVcrSideFunction::testIt()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.setGame(new game::Game());
    addDefaultBattle(session);

    // Test basic properties
    game::interface::VcrSideFunction testee(0, session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList());
    interpreter::test::ValueVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();

    TS_ASSERT_EQUALS(testee.getDimension(0), 1);
    TS_ASSERT_EQUALS(testee.getDimension(1), 4);   // 3 units

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "testIt: get").verifyInteger("ID", 30);
    }

    // Test failing invocation
    {
        // arity error
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }
    {
        // type error
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }
    {
        // range error
        afl::data::Segment seg;
        seg.pushBackInteger(4);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }
    {
        // range error
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }

    // Test invocation with null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() == 0);
    }

    // Test iteration
    {
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "testIt: get").verifyInteger("ID", 10);
    }

    // Test set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.set(args, 0), interpreter::Error);
    }
}

/** Test behaviour with empty environment. */
void
TestGameInterfaceVcrSideFunction::testEmpty()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;

    // No battles
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setShipList(new game::spec::ShipList());
        session.setGame(new game::Game());

        game::interface::VcrSideFunction testee(0, session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList());
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() == 0);
    }

    // Empty battle database
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setShipList(new game::spec::ShipList());
        session.setGame(new game::Game());

        session.getGame()->currentTurn().setBattles(new game::vcr::test::Database());

        game::interface::VcrSideFunction testee(0, session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList());
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() == 0);
    }
}

