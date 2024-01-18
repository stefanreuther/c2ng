/**
  *  \file test/game/interface/plugincontexttest.cpp
  *  \brief Test for game::interface::PluginContext
  */

#include "game/interface/plugincontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"

namespace {
    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;

        Environment()
            : tx(), fs(), session(tx, fs)
            { session.plugins().addNewPlugin(new util::plugin::Plugin("T")); }
    };
}

/** Test functions of the context. */
AFL_TEST("game.interface.PluginContext:basics", a)
{
    // Environment
    Environment env;

    // Object under test
    game::interface::PluginContext testee("T", env.session);

    // Verify some properties
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyTypes();
    verif.verifyBasics();
    verif.verifyNotSerializable();
    verif.verifyString("ID", "T");

    // Other attributes
    a.checkNull("01. getObject", testee.getObject());
    a.checkEqual("02. toString", testee.toString(true), "System.Plugin(\"T\")");
}

/** Test operation with nonexistant plugin.
    This is a disallowed state (we only create PluginContext for existing plugins),
    but could occur if someone keeps a PluginContext object for a very long time. */
AFL_TEST("game.interface.PluginContext:missing-plugin", a)
{
    Environment env;
    game::interface::PluginContext testee("Q", env.session);

    // Must report null property value
    interpreter::test::ContextVerifier(testee, a).verifyNull("ID");
}

/** Test creation: regular case. */
AFL_TEST("game.interface.PluginContext:IFSystemPlugin:normal", a)
{
    Environment env;

    // A single string parameter
    afl::data::Segment seg;
    seg.pushBackString("t");         // can be lowercase!
    interpreter::Arguments args(seg, 0, 1);

    // Test
    std::auto_ptr<afl::data::Value> p(game::interface::IFSystemPlugin(env.session, args));

    // Result must be a PluginContext
    game::interface::PluginContext* ctx = dynamic_cast<game::interface::PluginContext*>(p.get());
    a.checkNonNull("ctx", ctx);
    interpreter::test::ContextVerifier(*ctx, a).verifyString("ID", "T");
}

/** Test creation: null parameter. */
AFL_TEST("game.interface.PluginContext:IFSystemPlugin:null", a)
{
    Environment env;

    // A single null parameter
    afl::data::Segment seg;
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 1);

    // Test
    std::auto_ptr<afl::data::Value> p(game::interface::IFSystemPlugin(env.session, args));

    // Result must be null
    a.checkNull("result", p.get());
}

/** Test creation: unknown name. */
AFL_TEST("game.interface.PluginContext:IFSystemPlugin:unknown-name", a)
{
    Environment env;

    // A single string parameter, nonexistant ID
    afl::data::Segment seg;
    seg.pushBackString("qq");
    interpreter::Arguments args(seg, 0, 1);

    // Test
    std::auto_ptr<afl::data::Value> p(game::interface::IFSystemPlugin(env.session, args));

    // Result must be null
    a.checkNull("result", p.get());
}

/** Test creation: error cases. */

// No parameters
AFL_TEST("game.interface.PluginContext:IFSystemPlugin:error:too-few-parameters", a)
{
    Environment env;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFSystemPlugin(env.session, args), interpreter::Error);
}

// Too many parameters
AFL_TEST("game.interface.PluginContext:IFSystemPlugin:error:too-many-parameters", a)
{
    Environment env;
    afl::data::Segment seg;
    seg.pushBackString("X");
    seg.pushBackString("Y");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFSystemPlugin(env.session, args), interpreter::Error);
}
