/**
  *  \file u/t_game_interface_missionfunction.cpp
  *  \brief Test for game::interface::MissionFunction
  */

#include "game/interface/missionfunction.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/session.hpp"
#include "game/spec/mission.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

/** Test standard cases. */
void
TestGameInterfaceMissionFunction::testIt()
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
    interpreter::test::ValueVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();
    TS_ASSERT_EQUALS(testee.getDimension(0), 0);

    // Good case: "Mission(8)" -> intercept
    {
        afl::data::Segment seg;
        seg.pushBackInteger(8);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "mission(8)").verifyString("NAME", "Intercept");
    }

    // Good case: "Mission(9,1)" -> rob ship
    {
        afl::data::Segment seg;
        seg.pushBackInteger(9);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 2);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "mission(9,1)").verifyString("NAME", "Rob Ship");
    }

    // Good case: "Mission(9,5)" -> empty, because player 5 has mission 1 which is not defined
    {
        afl::data::Segment seg;
        seg.pushBackInteger(9);
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, 2);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() == 0);
    }

    // Good case: "Mission(9)" -> plunder, because that is the first mission 9
    {
        afl::data::Segment seg;
        seg.pushBackInteger(9);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "mission(9)").verifyString("NAME", "Plunder");
    }

    // Border case: null
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() == 0);
    }

    // Error case: type error
    {
        afl::data::Segment seg;
        seg.pushBackString("8");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }

    // Error case: arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }

    // Iteration
    {
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "first").verifyString("NAME", "Intercept");
    }

    // Set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.set(args, 0), interpreter::Error);
    }
}

/** Test missing objects. */
void
TestGameInterfaceMissionFunction::testNull()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;

    // No root, but empty ship list
    {
        game::Session session(tx, fs);
        session.setShipList(new game::spec::ShipList());

        game::interface::MissionFunction testee(session);
        interpreter::test::verifyNewNull("no root first", testee.makeFirstContext());

        afl::data::Segment seg;
        seg.pushBackInteger(8);
        interpreter::Arguments args(seg, 0, 1);
        interpreter::test::verifyNewNull("no root call", testee.get(args));
    }

    // No ship list, but empty root
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());

        game::interface::MissionFunction testee(session);
        interpreter::test::verifyNewNull("no ship list first", testee.makeFirstContext());

        afl::data::Segment seg;
        seg.pushBackInteger(8);
        interpreter::Arguments args(seg, 0, 1);
        interpreter::test::verifyNewNull("no ship list call", testee.get(args));
    }
}

