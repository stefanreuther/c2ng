/**
  *  \file test/game/interface/missionfunctiontest.cpp
  *  \brief Test for game::interface::MissionFunction
  */

#include "game/interface/missionfunction.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "game/spec/mission.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** Test standard cases. */
AFL_TEST("game.interface.MissionFunction:basics", a)
{
    // Session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Content
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.getRoot()->hostConfiguration()[game::config::HostConfiguration::PlayerSpecialMission].set("5,4,6,1,2");

    session.setShipList(new game::spec::ShipList());
    game::spec::MissionList& ml = session.getShipList()->missions();
    ml.addMission(game::spec::Mission(8, "!is*,Intercept"));
    ml.addMission(game::spec::Mission(9, "+4,Plunder"));
    ml.addMission(game::spec::Mission(9, "+5,Rob Ship"));

    // Testee
    game::interface::MissionFunction testee(session);
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();
    a.checkEqual("01. getDimension", testee.getDimension(0), 0U);

    // Good case: "Mission(8)" -> intercept
    {
        afl::data::Segment seg;
        seg.pushBackInteger(8);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNonNull("11. get(8)", result.get());
        interpreter::test::ContextVerifier(*result, a("12. get(8)")).verifyString("NAME", "Intercept");
    }

    // Good case: "Mission(9,1)" -> rob ship
    {
        afl::data::Segment seg;
        seg.pushBackInteger(9);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 2);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNonNull("21. get(9,1)", result.get());
        interpreter::test::ContextVerifier(*result, a("22. get(9,1)")).verifyString("NAME", "Rob Ship");
    }

    // Good case: "Mission(9,5)" -> empty, because player 5 has mission 1 which is not defined
    {
        afl::data::Segment seg;
        seg.pushBackInteger(9);
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, 2);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNull("31. get(9,5)", result.get());
    }

    // Good case: "Mission(9)" -> plunder, because that is the first mission 9
    {
        afl::data::Segment seg;
        seg.pushBackInteger(9);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNonNull("41. get(9)", result.get());
        interpreter::test::ContextVerifier(*result, a("42. get(9)")).verifyString("NAME", "Plunder");
    }

    // Border case: null
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNull("51. null", result.get());
    }

    // Error case: type error
    {
        afl::data::Segment seg;
        seg.pushBackString("8");
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("61. type error"), testee.get(args), interpreter::Error);
    }

    // Error case: arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("71. arity error"), testee.get(args), interpreter::Error);
    }

    // Iteration
    {
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        a.checkNonNull("81. makeFirstContext", result.get());
        interpreter::test::ContextVerifier(*result, a("82. makeFirstContext")).verifyString("NAME", "Intercept");
    }

    // Set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("91. set"), testee.set(args, 0), interpreter::Error);
    }
}

// No root, but empty ship list
AFL_TEST("game.interface.MissionFunction:no-root", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());

    game::interface::MissionFunction testee(session);
    interpreter::test::verifyNewNull(a("makeFirstContext"), testee.makeFirstContext());

    afl::data::Segment seg;
    seg.pushBackInteger(8);
    interpreter::Arguments args(seg, 0, 1);
    interpreter::test::verifyNewNull(a("get"), testee.get(args));
}

// No ship list, but empty root
AFL_TEST("game.interface.MissionFunction:no-shiplist", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());

    game::interface::MissionFunction testee(session);
    interpreter::test::verifyNewNull(a("makeFirstContext"), testee.makeFirstContext());

    afl::data::Segment seg;
    seg.pushBackInteger(8);
    interpreter::Arguments args(seg, 0, 1);
    interpreter::test::verifyNewNull(a("get"), testee.get(args));
}
