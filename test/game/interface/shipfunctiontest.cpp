/**
  *  \file test/game/interface/shipfunctiontest.cpp
  *  \brief Test for game::interface::ShipFunction
  */

#include "game/interface/shipfunction.hpp"

#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

namespace {
    void addShipXY(game::Session& session, game::Id_t id)
    {
        game::map::Ship& sh = *session.getGame()->currentTurn().universe().ships().create(id);
        sh.addShipXYData(game::map::Point(1000, 1000), 1, 100, game::PlayerSet_t(2));
        sh.internalCheck(game::PlayerSet_t(2), 10);
    }
}

/** General tests. */
AFL_TEST("game.interface.ShipFunction:basics", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    session.setShipList(new game::spec::ShipList());

    addShipXY(session, 100);

    // Test basic properties
    game::interface::ShipFunction testee(session);
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

    a.checkEqual("01. getDimension 0", testee.getDimension(0), 1);
    a.checkEqual("02. getDimension 1", testee.getDimension(1), 101);     // last ship Id, plus 1

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(100);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNonNull("11. get", result.get());
        interpreter::test::ContextVerifier(*result, a("12. get")).verifyInteger("ID", 100);
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

    // Undefined ship / range error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(6);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNull("31. range error", result.get());
    }
    {
        afl::data::Segment seg;
        seg.pushBackInteger(6666);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNull("32. range error", result.get());
    }

    // Test invocation with null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNull("41. null", result.get());
    }

    // Test iteration
    {
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        a.checkNonNull("51. makeFirstContext", result.get());
        interpreter::test::ContextVerifier(*result, a("52. makeFirstContext")).verifyInteger("ID", 100);
    }

    // Test set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("61. set"), testee.set(args, 0), interpreter::Error);
    }
}

// Empty session
AFL_TEST("game.interface.ShipFunction:empty-session", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    game::interface::ShipFunction testee(session);
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    a.checkNull("01. get", result.get());

    a.checkEqual("11. getDimension 0", testee.getDimension(0), 1);
    a.checkEqual("12. getDimension 1", testee.getDimension(1), 0);
}

// Session populated with empty objects
AFL_TEST("game.interface.ShipFunction:empty-universe", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    session.setShipList(new game::spec::ShipList());

    game::interface::ShipFunction testee(session);
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    a.checkNull("01. get", result.get());

    a.checkEqual("11. getDimension 0", testee.getDimension(0), 1);
    a.checkEqual("12. getDimension 1", testee.getDimension(1), 1);
}
