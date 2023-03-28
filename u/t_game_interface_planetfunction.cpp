/**
  *  \file u/t_game_interface_planetfunction.cpp
  *  \brief Test for game::interface::PlanetFunction
  */

#include "game/interface/planetfunction.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"

namespace {
    void addPlanetXY(game::Session& session, game::Id_t id, int x, int y)
    {
        game::map::Planet& pl = *session.getGame()->currentTurn().universe().planets().create(id);
        pl.setPosition(game::map::Point(x, y));
        pl.internalCheck(session.getGame()->mapConfiguration(), game::PlayerSet_t(), 10, session.translator(), session.log());
    }
}

/** General tests. */
void
TestGameInterfacePlanetFunction::testIt()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());

    addPlanetXY(session, 100, 1000, 1000);

    // Test basic properties
    game::interface::PlanetFunction testee(session);
    interpreter::test::ValueVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();

    TS_ASSERT_EQUALS(testee.getDimension(0), 1);
    TS_ASSERT_EQUALS(testee.getDimension(1), 101);     // last planet Id, plus 1

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(100);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "testIt: get").verifyInteger("ID", 100);
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

    // Undefined planet / range error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(6);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() == 0);
    }
    {
        afl::data::Segment seg;
        seg.pushBackInteger(6666);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() == 0);
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
        interpreter::test::ContextVerifier(*result, "testIt: get").verifyInteger("ID", 100);
    }

    // Test set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.set(args, 0), interpreter::Error);
    }
}

/** Test behaviour on empty session. */
void
TestGameInterfacePlanetFunction::testNull()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;

    // Empty session
    {
        game::Session session(tx, fs);

        game::interface::PlanetFunction testee(session);
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() == 0);

        TS_ASSERT_EQUALS(testee.getDimension(0), 1);
        TS_ASSERT_EQUALS(testee.getDimension(1), 0);
    }

    // Session populated with empty objects
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setGame(new game::Game());

        game::interface::PlanetFunction testee(session);
        std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
        TS_ASSERT(result.get() == 0);

        TS_ASSERT_EQUALS(testee.getDimension(0), 1);
        TS_ASSERT_EQUALS(testee.getDimension(1), 1);
    }
}

