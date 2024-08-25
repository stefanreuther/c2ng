/**
  *  \file test/game/interface/ionstormfunctiontest.cpp
  *  \brief Test for game::interface::IonStormFunction
  */

#include "game/interface/ionstormfunction.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/ionstorm.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

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

AFL_TEST("game.interface.IonStormFunction:basic", a)
{
    // Environment
    Environment env;
    addStorm(env, 20, "Twenty");
    addStorm(env, 30, "Thirty");

    // Test basic properties
    game::interface::IonStormFunction testee(env.session);
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

    a.checkEqual("01. getDimension 0", testee.getDimension(0), 1U);
    a.checkEqual("02. getDimension 1", testee.getDimension(1), 31U);

    // Test successful invocation
    {
        afl::data::Segment seg;
        seg.pushBackInteger(20);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNonNull("11. get", result.get());
        interpreter::test::ContextVerifier(*result, a("12. get")).verifyInteger("ID", 20);
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

    // Out of range
    {
        // range error
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<interpreter::Context> result(testee.get(args));
        a.checkNull("31. range error", result.get());
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
        interpreter::test::ContextVerifier(*result, a("52. makeFirstContext")).verifyInteger("ID", 20);
    }

    // Test set
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("61. set"), testee.set(args, 0), interpreter::Error);
    }
}

AFL_TEST("game.interface.IonStormFunction:empty-universe", a)
{
    Environment env;
    game::interface::IonStormFunction testee(env.session);

    // Inquiry
    a.checkEqual("01. getDimension 0", testee.getDimension(0), 1U);
    a.checkEqual("02. getDimension 1", testee.getDimension(1), 1U);

    // Test iteration
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    a.checkNull("11. makeFirstContext", result.get());
}

AFL_TEST("game.interface.IonStormFunction:empty-session", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    game::interface::IonStormFunction testee(session);

    // Inquiry
    a.checkEqual("01. getDimension 0", testee.getDimension(0), 1U);
    a.checkEqual("02. getDimension 1", testee.getDimension(1), 0U);

    // Test iteration
    std::auto_ptr<interpreter::Context> result(testee.makeFirstContext());
    a.checkNull("11. makeFirstContext", result.get());
}
