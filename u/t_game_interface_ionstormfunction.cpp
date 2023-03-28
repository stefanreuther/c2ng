/**
  *  \file u/t_game_interface_ionstormfunction.cpp
  *  \brief Test for game::interface::IonStormFunction
  */

#include "game/interface/ionstormfunction.hpp"

#include "t_game_interface.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "game/session.hpp"
#include "game/game.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "game/map/universe.hpp"
#include "game/map/ionstorm.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"

namespace {
    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;

        Environment()
            : tx(), fs(), session(tx, fs)
            {
                session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
                session.setGame(new game::Game());
           }
    };

    game::map::IonStorm& addStorm(Environment& env, game::Id_t id, String_t name)
    {
        game::map::IonStorm& st = *env.session.getGame()->currentTurn().universe().ionStorms().create(id);
        st.setName(name);
        st.setVoltage(20);
        return st;
    }
}

void
TestGameInterfaceIonStormFunction::testIt()
{
    // Environment
    Environment env;
    addStorm(env, 20, "Twenty");
    addStorm(env, 30, "Thirty");

    // Test basic properties
    game::interface::IonStormFunction testee(env.session);
    interpreter::test::ValueVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();

    TS_ASSERT_EQUALS(testee.getDimension(0), 1);
    TS_ASSERT_EQUALS(testee.getDimension(1), 31);

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(20);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        TS_ASSERT(result.get() != 0);
        interpreter::test::ContextVerifier(*result, "testIt: get").verifyInteger("ID", 20);
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

    // Out of range
    {
        // range error
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
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
        interpreter::test::ContextVerifier(*result, "testIt: get").verifyInteger("ID", 20);
    }

    // Test set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.set(args, 0), interpreter::Error);
    }
}

void
TestGameInterfaceIonStormFunction::testEmptyUniverse()
{
    Environment env;
    game::interface::IonStormFunction testee(env.session);

    // Inquiry
    TS_ASSERT_EQUALS(testee.getDimension(0), 1);
    TS_ASSERT_EQUALS(testee.getDimension(1), 1);

    // Test iteration
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    TS_ASSERT(result.get() == 0);
}

void
TestGameInterfaceIonStormFunction::testEmptySession()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    game::interface::IonStormFunction testee(session);

    // Inquiry
    TS_ASSERT_EQUALS(testee.getDimension(0), 1);
    TS_ASSERT_EQUALS(testee.getDimension(1), 0);

    // Test iteration
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    TS_ASSERT(result.get() == 0);
}

