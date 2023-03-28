/**
  *  \file u/t_game_interface_hullcontext.cpp
  *  \brief Test for game::interface::HullContext
  */

#include "game/interface/hullcontext.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "interpreter/test/contextverifier.hpp"

using game::spec::Cost;

/** Test basics: general behaviour, specific properties. */
void
TestGameInterfaceHullContext::testBasics()
{
    // Environment
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    game::spec::Hull& h = *shipList->hulls().create(3);
    h.setName("Orville");
    h.setTechLevel(9);
    h.setShortName("Oh.");
    h.setMaxBeams(7);
    h.setMaxCargo(200);
    h.setMaxFuel(150);
    h.setMaxCrew(20);
    h.setNumEngines(2);
    h.setNumBays(6);
    h.setMaxLaunchers(9);
    h.setExternalPictureNumber(11);
    h.setInternalPictureNumber(22);
    h.cost().set(Cost::Tritanium, 5);
    h.cost().set(Cost::Duranium, 7);
    h.cost().set(Cost::Molybdenum, 9);
    h.cost().set(Cost::Money, 11);
    h.cost().set(Cost::Supplies, 13);

    // Instance
    game::interface::HullContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, "testBasics");
    verif.verifyBasics();
    verif.verifySerializable(interpreter::TagNode::Tag_Hull, 3, afl::base::Nothing);
    verif.verifyTypes();
    TS_ASSERT(testee.getObject() == 0);

    // Specific properties
    TS_ASSERT_EQUALS(testee.toString(true), "Hull(3)");
    verif.verifyInteger("TECH", 9);
    verif.verifyInteger("COST.D", 7);
    verif.verifyString("NAME", "Orville");
    verif.verifyString("SPECIAL", "");
}

/** Test iteration. */
void
TestGameInterfaceHullContext::testIteration()
{
    // Given an environment with multiple hulls...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    shipList->hulls().create(3)->setName("Three");
    shipList->hulls().create(5)->setName("Five");
    shipList->hulls().create(6)->setName("Six");

    // ...I expect to be able to iterate through them using Context methods.
    game::interface::HullContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, "testIteration");
    verif.verifyString("NAME", "Three");
    TS_ASSERT(testee.next());
    verif.verifyString("NAME", "Five");
    TS_ASSERT(testee.next());
    verif.verifyString("NAME", "Six");
    TS_ASSERT(!testee.next());
}

/** Test behaviour on non-existant object.
    Normally, such a HullContext instance cannot be created. */
void
TestGameInterfaceHullContext::testNull()
{
    // Given an environment with no hulls...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();

    // ...I expect HullContext to report all properties as null...
    game::interface::HullContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, "testNull");
    verif.verifyNull("NAME");
    verif.verifyNull("TECH");

    // ...and nothing to be assignable.
    TS_ASSERT_THROWS(verif.setStringValue("NAME", "x"), interpreter::Error);
}

/** Test creation using factory function. */
void
TestGameInterfaceHullContext::testCreate()
{
    // Given an environment with one hull...
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.getShipList()->hulls().create(3)->setName("Three");

    // ...I expect to be able to create a HullContext for it...
    {
        std::auto_ptr<game::interface::HullContext> p(game::interface::HullContext::create(3, session));
        TS_ASSERT(p.get() != 0);
        interpreter::test::ContextVerifier(*p, "testCreate").verifyString("NAME", "Three");
    }

    // ...but not for any other Id.
    {
        std::auto_ptr<game::interface::HullContext> p(game::interface::HullContext::create(0, session));
        TS_ASSERT(p.get() == 0);
    }
    {
        std::auto_ptr<game::interface::HullContext> p(game::interface::HullContext::create(10, session));
        TS_ASSERT(p.get() == 0);
    }
}

/** Test set(). */
void
TestGameInterfaceHullContext::testSet()
{
    // Given an environment with a hull...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    shipList->hulls().create(3)->setName("Three");

    // ...I expect to be able to change the Name property...
    game::interface::HullContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, "testSet");
    TS_ASSERT_THROWS_NOTHING(verif.setStringValue("NAME", "New"));
    TS_ASSERT_THROWS_NOTHING(verif.setIntegerValue("IMAGE", 555));
    TS_ASSERT_EQUALS(shipList->hulls().get(3)->getName(shipList->componentNamer()), "New");
    TS_ASSERT_EQUALS(shipList->hulls().get(3)->getInternalPictureNumber(), 555);

    // ...but not the Id or other properties.
    TS_ASSERT_THROWS(verif.setIntegerValue("ID", 8), interpreter::Error);
    TS_ASSERT_THROWS(verif.setIntegerValue("TECH", 8), interpreter::Error);
}
