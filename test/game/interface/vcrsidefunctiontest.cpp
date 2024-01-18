/**
  *  \file test/game/interface/vcrsidefunctiontest.cpp
  *  \brief Test for game::interface::VcrSideFunction
  */

#include "game/interface/vcrsidefunction.hpp"

#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/vcr/test/battle.hpp"
#include "game/vcr/test/database.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

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

AFL_TEST("game.interface.VcrSideFunction:basics", a)
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
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

    a.checkEqual("01. getDimension 0", testee.getDimension(0), 1);
    a.checkEqual("02. getDimension 1", testee.getDimension(1), 4);   // 3 units

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNonNull("11. get", result.get());
        interpreter::test::ContextVerifier(*result, a("12. get")).verifyInteger("ID", 30);
    }

    // Test failing invocation
    {
        // arity error
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("21. arity error"), testee.get(args), interpreter::Error);
    }
    {
        // type error
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("22. type error"), testee.get(args), interpreter::Error);
    }
    {
        // range error
        afl::data::Segment seg;
        seg.pushBackInteger(4);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("23. range error"), testee.get(args), interpreter::Error);
    }
    {
        // range error
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("24. range error"), testee.get(args), interpreter::Error);
    }

    // Test invocation with null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNull("31. null", result.get());
    }

    // Test iteration
    {
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        a.checkNonNull("41. makeFirstContext", result.get());
        interpreter::test::ContextVerifier(*result, a("42. makeFirstContext")).verifyInteger("ID", 10);
    }

    // Test set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("51. set"), testee.set(args, 0), interpreter::Error);
    }
}

// No battles
AFL_TEST("game.interface.VcrSideFunction:error:no-battles", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.setGame(new game::Game());

    game::interface::VcrSideFunction testee(0, session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList());
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    a.checkNull("ctx", result.get());
}

// Empty battle database
AFL_TEST("game.interface.VcrSideFunction:error:empty-battles", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.setGame(new game::Game());

    session.getGame()->currentTurn().setBattles(new game::vcr::test::Database());

    game::interface::VcrSideFunction testee(0, session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList());
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    a.checkNull("ctx", result.get());
}
