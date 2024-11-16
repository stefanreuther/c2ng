/**
  *  \file test/game/interface/ionstormcontexttest.cpp
  *  \brief Test for game::interface::IonStormContext
  */

#include "game/interface/ionstormcontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.interface.IonStormContext:basics", a)
{
    Environment env;
    game::map::IonStorm& st = addStorm(env, ID, "Fred");
    addStorm(env, ID+1, "Barney");

    // Instance
    game::interface::IonStormContext testee(ID, env.session, env.session.getGame()->viewpointTurn());
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifySerializable(interpreter::TagNode::Tag_Ion, ID, afl::base::Nothing);
    verif.verifyTypes();
    a.checkEqual("01. getObject", testee.getObject(), &st);

    // Specific properties
    verif.verifyInteger("ID", ID);
    verif.verifyString("NAME", "Fred");

    // Iteration
    a.check("11. next", testee.next());
    verif.verifyString("NAME", "Barney");
    a.check("12. next", !testee.next());
}

/** Test property modification. */
AFL_TEST("game.interface.IonStormContext:set", a)
{
    Environment env;
    addStorm(env, ID, "Fred");

    // Property access fails
    game::interface::IonStormContext testee(ID, env.session, env.session.getGame()->viewpointTurn());
    interpreter::test::ContextVerifier verif(testee, a);
    AFL_CHECK_THROWS(a("01. LOC.X"), verif.setIntegerValue("LOC.X", 1000), interpreter::Error);
    AFL_CHECK_THROWS(a("02. MARK"), verif.setIntegerValue("MARK", 1000), interpreter::Error);
}

/** Test usage of commands. */
AFL_TEST("game.interface.IonStormContext:command", a)
{
    Environment env;
    game::map::IonStorm& st = addStorm(env, ID, "Fred");
    a.check("01. isMarked", !st.isMarked());

    // Retrieve
    game::interface::IonStormContext testee(ID, env.session, env.session.getGame()->viewpointTurn());
    std::auto_ptr<afl::data::Value> meth(interpreter::test::ContextVerifier(testee, a).getValue("MARK"));

    // Invoke as command
    interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(meth.get());
    a.checkNonNull("11. CallableValue", cv);
    interpreter::test::ValueVerifier(*cv, a).verifyBasics();
    {
        afl::data::Segment seg;
        interpreter::Process proc(env.session.world(), "dummy", 1);
        AFL_CHECK_SUCCEEDS(a("12. call"), cv->call(proc, seg, false));
    }

    // Verify that command was executed
    a.check("21. isMarked", st.isMarked());
}

/** Test factory function. */
// Success case
AFL_TEST("game.interface.IonStormContext:create:success", a)
{
    Environment env;
    game::map::IonStorm& st = addStorm(env, ID, "Fred");
    std::auto_ptr<game::interface::IonStormContext> ctx(game::interface::IonStormContext::create(ID, env.session, env.session.getGame()->viewpointTurn()));
    a.checkNonNull("ctx", ctx.get());
    a.checkEqual("getObject", ctx->getObject(), &st);
}

// Failure case
AFL_TEST("game.interface.IonStormContext:create:bad-id", a)
{
    Environment env;
    addStorm(env, ID, "Fred");
    std::auto_ptr<game::interface::IonStormContext> ctx(game::interface::IonStormContext::create(ID+1, env.session, env.session.getGame()->viewpointTurn()));
    a.checkNull("ctx", ctx.get());
}

/** Test accessing an empty/undefined ion storm. */
AFL_TEST("game.interface.IonStormContext:null", a)
{
    Environment env;
    game::interface::IonStormContext testee(ID, env.session, env.session.getGame()->viewpointTurn());

    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyNull("ID");
    verif.verifyNull("NAME");

    AFL_CHECK_THROWS(a("01. set VOLTAGE"), verif.setIntegerValue("VOLTAGE", 10), interpreter::Error);
}
