/**
  *  \file u/t_game_interface_ionstormcontext.cpp
  *  \brief Test for game::interface::IonStormContext
  */

#include "game/interface/ionstormcontext.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/ionstorm.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/test/contextverifier.hpp"

namespace {
    const int ID = 17;

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

/** Test basics: property retrieval, enumeration. */
void
TestGameInterfaceIonStormContext::testBasics()
{
    Environment env;
    game::map::IonStorm& st = addStorm(env, ID, "Fred");
    addStorm(env, ID+1, "Barney");

    // Instance
    game::interface::IonStormContext testee(ID, env.session, *env.session.getGame());
    interpreter::test::ContextVerifier verif(testee, "testBasics");
    verif.verifyBasics();
    verif.verifySerializable(interpreter::TagNode::Tag_Ion, ID, afl::base::Nothing);
    verif.verifyTypes();
    TS_ASSERT_EQUALS(testee.getObject(), &st);

    // Specific properties
    verif.verifyInteger("ID", ID);
    verif.verifyString("NAME", "Fred");

    // Iteration
    TS_ASSERT(testee.next());
    verif.verifyString("NAME", "Barney");
    TS_ASSERT(!testee.next());
}

/** Test property modification. */
void
TestGameInterfaceIonStormContext::testSet()
{
    Environment env;
    addStorm(env, ID, "Fred");

    // Property access fails
    game::interface::IonStormContext testee(ID, env.session, *env.session.getGame());
    interpreter::test::ContextVerifier verif(testee, "testSet");
    TS_ASSERT_THROWS(verif.setIntegerValue("LOC.X", 1000), interpreter::Error);
    TS_ASSERT_THROWS(verif.setIntegerValue("MARK", 1000), interpreter::Error);
}

/** Test usage of commands. */
void
TestGameInterfaceIonStormContext::testCommand()
{
    Environment env;
    game::map::IonStorm& st = addStorm(env, ID, "Fred");
    TS_ASSERT(!st.isMarked());

    // Retrieve
    game::interface::IonStormContext testee(ID, env.session, *env.session.getGame());
    std::auto_ptr<afl::data::Value> meth(interpreter::test::ContextVerifier(testee, "testCommand").getValue("MARK"));

    // Invoke as command
    interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(meth.get());
    TS_ASSERT(cv != 0);
    interpreter::test::ValueVerifier(*cv, "testCommand").verifyBasics();
    {
        afl::data::Segment seg;
        interpreter::Process proc(env.session.world(), "dummy", 1);
        TS_ASSERT_THROWS_NOTHING(cv->call(proc, seg, false));
    }

    // Verify that command was executed
    TS_ASSERT(st.isMarked());
}

/** Test factory function. */
void
TestGameInterfaceIonStormContext::testCreate()
{
    Environment env;
    game::map::IonStorm& st = addStorm(env, ID, "Fred");

    // Success case
    {
        std::auto_ptr<game::interface::IonStormContext> ctx(game::interface::IonStormContext::create(ID, env.session));
        TS_ASSERT(ctx.get() != 0);
        TS_ASSERT_EQUALS(ctx->getObject(), &st);
    }

    // Failure case
    {
        std::auto_ptr<game::interface::IonStormContext> ctx(game::interface::IonStormContext::create(ID+1, env.session));
        TS_ASSERT(ctx.get() == 0);
    }
}

/** Test factory function, empty session case. */
void
TestGameInterfaceIonStormContext::testCreateEmpty()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    std::auto_ptr<game::interface::IonStormContext> ctx(game::interface::IonStormContext::create(ID+1, session));
    TS_ASSERT(ctx.get() == 0);
}

/** Test accessing an empty/undefined ion storm. */
void
TestGameInterfaceIonStormContext::testAccessEmpty()
{
    Environment env;
    game::interface::IonStormContext testee(ID, env.session, *env.session.getGame());

    interpreter::test::ContextVerifier verif(testee, "testAccessEmpty");
    verif.verifyNull("ID");
    verif.verifyNull("NAME");

    TS_ASSERT_THROWS(verif.setIntegerValue("VOLTAGE", 10), interpreter::Error);
}

