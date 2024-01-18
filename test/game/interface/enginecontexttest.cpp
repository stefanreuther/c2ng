/**
  *  \file test/game/interface/enginecontexttest.cpp
  *  \brief Test for game::interface::EngineContext
  */

#include "game/interface/enginecontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/test/contextverifier.hpp"

/** Test engine basics. */
AFL_TEST("game.interface.EngineContext:basics", a)
{
    // Create ship list with an engine
    const int NR = 7;
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    game::spec::Engine* eng = shipList->engines().create(NR);
    a.checkNonNull("01. create", eng);

    // Populate the engine
    eng->setFuelFactor(1, 999);
    eng->setName("The Kettle");
    eng->setTechLevel(3);
    eng->cost().set(game::spec::Cost::Money, 500);

    // Verify
    game::interface::EngineContext ctx(NR, shipList);
    interpreter::test::ContextVerifier v(ctx, a);
    v.verifyTypes();
    v.verifyBasics();
    v.verifySerializable(interpreter::TagNode::Tag_Engine, NR, afl::base::Nothing);
    a.checkNull("11. getObject", ctx.getObject());

    // Verify individual properties
    a.checkEqual("21", ctx.toString(true), "Engine(7)");
    v.verifyInteger("ID", NR);
    v.verifyInteger("TECH", 3);
    v.verifyInteger("COST.MC", 500);
    v.verifyString("NAME", "The Kettle");
}

/** Test iteration. */
AFL_TEST("game.interface.EngineContext:iteration", a)
{
    // Given an environment with multiple engines...
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    shipList->engines().create(3)->setName("Three");
    shipList->engines().create(5)->setName("Five");
    shipList->engines().create(6)->setName("Six");

    // ...I expect to be able to iterate through them using Context methods.
    game::interface::EngineContext testee(3, shipList);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyString("NAME", "Three");
    a.check("01. next", testee.next());
    verif.verifyString("NAME", "Five");
    a.check("02. next", testee.next());
    verif.verifyString("NAME", "Six");
    a.check("03. next", !testee.next());
}

/** Test behaviour on non-existant object.
    Normally, such an EngineContext instance cannot be created. */
AFL_TEST("game.interface.EngineContext:null", a)
{
    // Given an environment with no engines...
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();

    // ...I expect EngineContext to report all properties as null...
    game::interface::EngineContext testee(3, shipList);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyNull("NAME");
    verif.verifyNull("TECH");

    // ...and nothing to be assignable.
    AFL_CHECK_THROWS(a("assign NAME"), verif.setStringValue("NAME", "x"), interpreter::Error);
}

/** Test creation using factory function. */
AFL_TEST("game.interface.EngineContext:create", a)
{
    // Given an environment with one engine...
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.getShipList()->engines().create(3)->setName("Three");

    // ...I expect to be able to create an EngineContext for it...
    {
        std::auto_ptr<game::interface::EngineContext> p(game::interface::EngineContext::create(3, session));
        a.checkNonNull("01. create for correct Id", p.get());
        interpreter::test::ContextVerifier(*p, a("02. create")).verifyString("NAME", "Three");
    }

    // ...but not for any other Id.
    {
        std::auto_ptr<game::interface::EngineContext> p(game::interface::EngineContext::create(0, session));
        a.checkNull("11. create 0", p.get());
    }
    {
        std::auto_ptr<game::interface::EngineContext> p(game::interface::EngineContext::create(10, session));
        a.checkNull("12. create 10", p.get());
    }
}

/** Test set(). */
AFL_TEST("game.interface.EngineContext:set", a)
{
    // Given an environment with an engine...
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    shipList->engines().create(3)->setName("Three");

    // ...I expect to be able to change the Name property...
    game::interface::EngineContext testee(3, shipList);
    interpreter::test::ContextVerifier verif(testee, a);
    AFL_CHECK_SUCCEEDS(a("01. set NAME"), verif.setStringValue("NAME", "New"));
    AFL_CHECK_SUCCEEDS(a("02. set SPEED$"), verif.setIntegerValue("SPEED$", 7));
    a.checkEqual("03. getName", shipList->engines().get(3)->getName(shipList->componentNamer()), "New");
    a.checkEqual("04. getMaxEfficientWarp", shipList->engines().get(3)->getMaxEfficientWarp(), 7);

    // ...but not the Id or other properties.
    AFL_CHECK_THROWS(a("11. set ID"), verif.setIntegerValue("ID", 8), interpreter::Error);
    AFL_CHECK_THROWS(a("12. set TECH"), verif.setIntegerValue("TECH", 8), interpreter::Error);
}
