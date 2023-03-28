/**
  *  \file u/t_game_interface_torpedocontext.cpp
  *  \brief Test for game::interface::TorpedoContext
  */

#include "game/interface/torpedocontext.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "interpreter/test/contextverifier.hpp"

using game::spec::Cost;

/** Test basics: general behaviour, specific properties. */
void
TestGameInterfaceTorpedoContext::testBasics()
{
    const int ID = 8;

    // Environment
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    game::spec::TorpedoLauncher& tl = *shipList->launchers().create(ID);
    tl.setName("Mudball");
    tl.setShortName("Splash!");
    tl.setMass(90);
    tl.setTechLevel(7);
    tl.setDamagePower(44);
    tl.cost().set(Cost::Tritanium, 5);
    tl.cost().set(Cost::Duranium, 7);
    tl.cost().set(Cost::Molybdenum, 9);
    tl.cost().set(Cost::Money, 11);
    tl.cost().set(Cost::Supplies, 13);
    tl.torpedoCost().set(Cost::Tritanium, 1);
    tl.torpedoCost().set(Cost::Duranium, 2);
    tl.torpedoCost().set(Cost::Molybdenum, 3);
    tl.torpedoCost().set(Cost::Money, 4);
    tl.torpedoCost().set(Cost::Supplies, 5);

    // Torpedo instance
    {
        game::interface::TorpedoContext testee(false, ID, shipList, root);
        interpreter::test::ContextVerifier verif(testee, "testBasics: torpedo");
        verif.verifyBasics();
        verif.verifySerializable(interpreter::TagNode::Tag_Torpedo, ID, afl::base::Nothing);
        verif.verifyTypes();
        TS_ASSERT(testee.getObject() == 0);

        TS_ASSERT_EQUALS(testee.toString(true), "Torpedo(8)");
        verif.verifyInteger("TECH", 7);
        verif.verifyInteger("COST.D", 2);
        verif.verifyInteger("DAMAGE", 88);       // doubled, default host-config is non-alternative combat
        verif.verifyString("NAME", "Mudball");
    }

    // Launcher instance
    {
        game::interface::TorpedoContext testee(true, ID, shipList, root);
        interpreter::test::ContextVerifier verif(testee, "testBasics: launcher");
        verif.verifyBasics();
        verif.verifySerializable(interpreter::TagNode::Tag_Launcher, ID, afl::base::Nothing);
        verif.verifyTypes();
        TS_ASSERT(testee.getObject() == 0);

        TS_ASSERT_EQUALS(testee.toString(true), "Launcher(8)");
        verif.verifyInteger("TECH", 7);
        verif.verifyInteger("COST.D", 7);
        verif.verifyInteger("DAMAGE", 88);       // doubled, default host-config is non-alternative combat
        verif.verifyString("NAME", "Mudball");
    }
}

/** Test iteration. */
void
TestGameInterfaceTorpedoContext::testIteration()
{
    // Given an environment with multiple torpedo launchers...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    shipList->launchers().create(3)->setName("Three");
    shipList->launchers().create(5)->setName("Five");
    shipList->launchers().create(6)->setName("Six");

    // ...I expect to be able to iterate through them using Context methods.
    game::interface::TorpedoContext testee(false, 3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, "testIteration");
    verif.verifyString("NAME", "Three");
    TS_ASSERT(testee.next());
    verif.verifyString("NAME", "Five");
    TS_ASSERT(testee.next());
    verif.verifyString("NAME", "Six");
    TS_ASSERT(!testee.next());
}

/** Test behaviour on non-existant object.
    Normally, such a TorpedoContext instance cannot be created. */
void
TestGameInterfaceTorpedoContext::testNull()
{
    // Given an environment with no torpedo launchers...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();

    // ...I expect TorpedoContext to report all properties as null...
    game::interface::TorpedoContext testee(false, 3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, "testNull");
    verif.verifyNull("NAME");
    verif.verifyNull("TECH");

    // ...and nothing to be assignable.
    TS_ASSERT_THROWS(verif.setStringValue("NAME", "x"), interpreter::Error);
}

/** Test creation using factory function. */
void
TestGameInterfaceTorpedoContext::testCreate()
{
    const int ID = 8;

    // Given an environment with one torpedo launcher...
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    game::spec::TorpedoLauncher& tl = *session.getShipList()->launchers().create(ID);
    tl.setName("Three");
    tl.cost().set(Cost::Tritanium, 10);
    tl.torpedoCost().set(Cost::Tritanium, 20);

    // ...I expect to be able to create a TorpedoContext for it...
    {
        std::auto_ptr<game::interface::TorpedoContext> p(game::interface::TorpedoContext::create(false, ID, session));
        TS_ASSERT(p.get() != 0);
        interpreter::test::ContextVerifier(*p, "testCreate").verifyInteger("COST.T", 20);
    }
    {
        std::auto_ptr<game::interface::TorpedoContext> p(game::interface::TorpedoContext::create(true, ID, session));
        TS_ASSERT(p.get() != 0);
        interpreter::test::ContextVerifier(*p, "testCreate").verifyInteger("COST.T", 10);
    }

    // ...but not for any other Id.
    {
        std::auto_ptr<game::interface::TorpedoContext> p(game::interface::TorpedoContext::create(false, 0, session));
        TS_ASSERT(p.get() == 0);
    }
    {
        std::auto_ptr<game::interface::TorpedoContext> p(game::interface::TorpedoContext::create(false, 10, session));
        TS_ASSERT(p.get() == 0);
    }
}

/** Test set(). */
void
TestGameInterfaceTorpedoContext::testSet()
{
    const int ID = 7;

    // Given an environment with a beam...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    shipList->launchers().create(ID)->setName("Three");

    // ...I expect to be able to change the Name property...
    game::interface::TorpedoContext testee(true, ID, shipList, root);
    interpreter::test::ContextVerifier verif(testee, "testSet");
    TS_ASSERT_THROWS_NOTHING(verif.setStringValue("NAME", "New"));
    TS_ASSERT_EQUALS(shipList->launchers().get(ID)->getName(shipList->componentNamer()), "New");

    // ...but not the Id or other properties.
    TS_ASSERT_THROWS(verif.setIntegerValue("ID", 8), interpreter::Error);
    TS_ASSERT_THROWS(verif.setIntegerValue("DAMAGE", 8), interpreter::Error);
}

