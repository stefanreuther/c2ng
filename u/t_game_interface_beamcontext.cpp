/**
  *  \file u/t_game_interface_beamcontext.cpp
  *  \brief Test for game::interface::BeamContext
  */

#include "game/interface/beamcontext.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "interpreter/test/contextverifier.hpp"

using game::spec::Cost;

/** Test basics: general behaviour, specific properties. */
void
TestGameInterfaceBeamContext::testBasics()
{
    // Environment
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    game::spec::Beam& b = *shipList->beams().create(3);
    b.setName("Death ray");
    b.setShortName("Bzzz!");
    b.setMass(10);
    b.setTechLevel(3);
    b.setDamagePower(99);
    b.cost().set(Cost::Tritanium, 5);
    b.cost().set(Cost::Duranium, 7);
    b.cost().set(Cost::Molybdenum, 9);
    b.cost().set(Cost::Money, 11);
    b.cost().set(Cost::Supplies, 13);

    // Instance
    game::interface::BeamContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, "testBasics");
    verif.verifyBasics();
    verif.verifySerializable(interpreter::TagNode::Tag_Beam, 3, afl::base::Nothing);
    verif.verifyTypes();
    TS_ASSERT(testee.getObject() == 0);

    // Specific properties
    TS_ASSERT_EQUALS(testee.toString(true), "Beam(3)");
    verif.verifyInteger("TECH", 3);
    verif.verifyInteger("COST.D", 7);
    verif.verifyInteger("DAMAGE", 99);
    verif.verifyString("NAME", "Death ray");
}

/** Test iteration. */
void
TestGameInterfaceBeamContext::testIteration()
{
    // Given an environment with multiple beams...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    shipList->beams().create(3)->setName("Three");
    shipList->beams().create(5)->setName("Five");
    shipList->beams().create(6)->setName("Six");

    // ...I expect to be able to iterate through them using Context methods.
    game::interface::BeamContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, "testIteration");
    verif.verifyString("NAME", "Three");
    TS_ASSERT(testee.next());
    verif.verifyString("NAME", "Five");
    TS_ASSERT(testee.next());
    verif.verifyString("NAME", "Six");
    TS_ASSERT(!testee.next());
}

/** Test behaviour on non-existant object.
    Normally, such a BeamContext instance cannot be created. */
void
TestGameInterfaceBeamContext::testNull()
{
    // Given an environment with no beams...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();

    // ...I expect BeamContext to report all properties as null...
    game::interface::BeamContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, "testNull");
    verif.verifyNull("NAME");
    verif.verifyNull("TECH");

    // ...and nothing to be assignable.
    TS_ASSERT_THROWS(verif.setStringValue("NAME", "x"), interpreter::Error);
}

/** Test creation using factory function. */
void
TestGameInterfaceBeamContext::testCreate()
{
    // Given an environment with one beam...
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.getShipList()->beams().create(3)->setName("Three");

    // ...I expect to be able to create a BeamContext for it...
    {
        std::auto_ptr<game::interface::BeamContext> p(game::interface::BeamContext::create(3, session));
        TS_ASSERT(p.get() != 0);
        interpreter::test::ContextVerifier(*p, "testCreate").verifyString("NAME", "Three");
    }

    // ...but not for any other Id.
    {
        std::auto_ptr<game::interface::BeamContext> p(game::interface::BeamContext::create(0, session));
        TS_ASSERT(p.get() == 0);
    }
    {
        std::auto_ptr<game::interface::BeamContext> p(game::interface::BeamContext::create(10, session));
        TS_ASSERT(p.get() == 0);
    }
}

/** Test set(). */
void
TestGameInterfaceBeamContext::testSet()
{
    // Given an environment with a beam...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    shipList->beams().create(3)->setName("Three");

    // ...I expect to be able to change the Name property...
    game::interface::BeamContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, "testSet");
    TS_ASSERT_THROWS_NOTHING(verif.setStringValue("NAME", "New"));
    TS_ASSERT_EQUALS(shipList->beams().get(3)->getName(shipList->componentNamer()), "New");

    // ...but not the Id or other properties.
    TS_ASSERT_THROWS(verif.setIntegerValue("ID", 8), interpreter::Error);
    TS_ASSERT_THROWS(verif.setIntegerValue("DAMAGE", 8), interpreter::Error);
}

