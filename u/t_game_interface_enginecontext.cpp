/**
  *  \file u/t_game_interface_enginecontext.cpp
  *  \brief Test for game::interface::EngineContext
  */

#include "game/interface/enginecontext.hpp"

#include "t_game_interface.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test engine basics. */
void
TestGameInterfaceEngineContext::testIt()
{
    // Create ship list with an engine
    const int NR = 7;
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    game::spec::Engine* eng = shipList->engines().create(NR);
    TS_ASSERT(eng != 0);

    // Populate the engine
    eng->setFuelFactor(1, 999);
    eng->setName("The Kettle");
    eng->setTechLevel(3);
    eng->cost().set(game::spec::Cost::Money, 500);

    // Verify
    game::interface::EngineContext ctx(NR, shipList);
    interpreter::test::ContextVerifier v(ctx, "testIt");
    v.verifyTypes();
    v.verifyBasics();
    v.verifySerializable(interpreter::TagNode::Tag_Engine, NR, afl::base::Nothing);
    TS_ASSERT(ctx.getObject() == 0);

    // Verify individual properties
    TS_ASSERT_EQUALS(ctx.toString(true), "Engine(7)");
    v.verifyInteger("ID", NR);
    v.verifyInteger("TECH", 3);
    v.verifyInteger("COST.MC", 500);
    v.verifyString("NAME", "The Kettle");
}

/** Test iteration. */
void
TestGameInterfaceEngineContext::testIteration()
{
    // Given an environment with multiple engines...
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    shipList->engines().create(3)->setName("Three");
    shipList->engines().create(5)->setName("Five");
    shipList->engines().create(6)->setName("Six");

    // ...I expect to be able to iterate through them using Context methods.
    game::interface::EngineContext testee(3, shipList);
    interpreter::test::ContextVerifier verif(testee, "testIteration");
    verif.verifyString("NAME", "Three");
    TS_ASSERT(testee.next());
    verif.verifyString("NAME", "Five");
    TS_ASSERT(testee.next());
    verif.verifyString("NAME", "Six");
    TS_ASSERT(!testee.next());
}

/** Test behaviour on non-existant object.
    Normally, such an EngineContext instance cannot be created. */
void
TestGameInterfaceEngineContext::testNull()
{
    // Given an environment with no engines...
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();

    // ...I expect EngineContext to report all properties as null...
    game::interface::EngineContext testee(3, shipList);
    interpreter::test::ContextVerifier verif(testee, "testNull");
    verif.verifyNull("NAME");
    verif.verifyNull("TECH");

    // ...and nothing to be assignable.
    TS_ASSERT_THROWS(verif.setStringValue("NAME", "x"), interpreter::Error);
}

/** Test creation using factory function. */
void
TestGameInterfaceEngineContext::testCreate()
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
        TS_ASSERT(p.get() != 0);
        interpreter::test::ContextVerifier(*p, "testCreate").verifyString("NAME", "Three");
    }

    // ...but not for any other Id.
    {
        std::auto_ptr<game::interface::EngineContext> p(game::interface::EngineContext::create(0, session));
        TS_ASSERT(p.get() == 0);
    }
    {
        std::auto_ptr<game::interface::EngineContext> p(game::interface::EngineContext::create(10, session));
        TS_ASSERT(p.get() == 0);
    }
}

/** Test set(). */
void
TestGameInterfaceEngineContext::testSet()
{
    // Given an environment with an engine...
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    shipList->engines().create(3)->setName("Three");

    // ...I expect to be able to change the Name property...
    game::interface::EngineContext testee(3, shipList);
    interpreter::test::ContextVerifier verif(testee, "testSet");
    TS_ASSERT_THROWS_NOTHING(verif.setStringValue("NAME", "New"));
    TS_ASSERT_THROWS_NOTHING(verif.setIntegerValue("SPEED$", 7));
    TS_ASSERT_EQUALS(shipList->engines().get(3)->getName(shipList->componentNamer()), "New");
    TS_ASSERT_EQUALS(shipList->engines().get(3)->getMaxEfficientWarp(), 7);

    // ...but not the Id or other properties.
    TS_ASSERT_THROWS(verif.setIntegerValue("ID", 8), interpreter::Error);
    TS_ASSERT_THROWS(verif.setIntegerValue("TECH", 8), interpreter::Error);
}

