/**
  *  \file u/t_game_interface_plugincontext.cpp
  *  \brief Test for game::interface::PluginContext
  */

#include "game/interface/plugincontext.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/nullstream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"

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
void
TestGameInterfacePluginContext::testIt()
{
    // Environment
    Environment env;

    // Object under test
    game::interface::PluginContext testee("T", env.session);

    // Verify some properties
    interpreter::test::ContextVerifier verif(testee, "testIt");
    verif.verifyTypes();
    verif.verifyString("ID", "T");

    // Other attributes
    TS_ASSERT(testee.getObject() == 0);
    TS_ASSERT_DIFFERS(testee.toString(false), "");
    TS_ASSERT_EQUALS(testee.toString(true), "System.Plugin(\"T\")");

    // Check store()
    interpreter::TagNode tag;
    afl::io::NullStream aux;
    interpreter::vmio::NullSaveContext ctx;
    TS_ASSERT_THROWS(testee.store(tag, aux, ctx), interpreter::Error);

    // Check clone()
    std::auto_ptr<interpreter::Context> v(testee.clone());
    TS_ASSERT(v.get() != 0);
    TS_ASSERT_EQUALS(v->toString(false), testee.toString(false));
}

/** Test operation with nonexistant plugin.
    This is a disallowed state (we only create PluginContext for existing plugins),
    but could occur if someone keeps a PluginContext object for a very long time. */
void
TestGameInterfacePluginContext::testNonExistant()
{
    Environment env;
    game::interface::PluginContext testee("Q", env.session);

    // Must report null property value
    interpreter::test::ContextVerifier(testee, "testNonExistant").verifyNull("ID");
}

/** Test creation: regular case. */
void
TestGameInterfacePluginContext::testCreateRegular()
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
    TS_ASSERT(ctx != 0);
    interpreter::test::ContextVerifier(*ctx, "testCreateRegular").verifyString("ID", "T");
}

/** Test creation: null parameter. */
void
TestGameInterfacePluginContext::testCreateNull()
{
    Environment env;

    // A single null parameter
    afl::data::Segment seg;
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 1);

    // Test
    std::auto_ptr<afl::data::Value> p(game::interface::IFSystemPlugin(env.session, args));

    // Result must be null
    TS_ASSERT(p.get() == 0);
}

/** Test creation: unknown name. */
void
TestGameInterfacePluginContext::testCreateUnknown()
{
    Environment env;

    // A single string parameter, nonexistant ID
    afl::data::Segment seg;
    seg.pushBackString("qq");
    interpreter::Arguments args(seg, 0, 1);

    // Test
    std::auto_ptr<afl::data::Value> p(game::interface::IFSystemPlugin(env.session, args));

    // Result must be null
    TS_ASSERT(p.get() == 0);
}

/** Test creation: error cases. */
void
TestGameInterfacePluginContext::testCreateErrors()
{
    Environment env;

    // No parameters
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFSystemPlugin(env.session, args), interpreter::Error);
    }

    // Too many parameters
    {
        afl::data::Segment seg;
        seg.pushBackString("X");
        seg.pushBackString("Y");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFSystemPlugin(env.session, args), interpreter::Error);
    }
}
