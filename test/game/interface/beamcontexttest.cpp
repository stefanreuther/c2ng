/**
  *  \file test/game/interface/beamcontexttest.cpp
  *  \brief Test for game::interface::BeamContext
  */

#include "game/interface/beamcontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "interpreter/test/contextverifier.hpp"

using game::spec::Cost;

/** Test basics: general behaviour, specific properties. */
AFL_TEST("game.interface.BeamContext:basics", a)
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
    b.setDescription("Fzzz!");

    // Instance
    game::interface::BeamContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifySerializable(interpreter::TagNode::Tag_Beam, 3, afl::base::Nothing);
    verif.verifyTypes();
    a.checkNull("getObject", testee.getObject());

    // Specific properties
    a.checkEqual("toString", testee.toString(true), "Beam(3)");
    verif.verifyInteger("TECH", 3);
    verif.verifyInteger("COST.D", 7);
    verif.verifyInteger("COST.SUP", 13);
    verif.verifyInteger("DAMAGE", 99);
    verif.verifyString("NAME", "Death ray");
    verif.verifyString("DESCRIPTION", "Fzzz!");
}

/** Test iteration. */
AFL_TEST("game.interface.BeamContext:iteration", a)
{
    // Given an environment with multiple beams...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    shipList->beams().create(3)->setName("Three");
    shipList->beams().create(5)->setName("Five");
    shipList->beams().create(6)->setName("Six");

    // ...I expect to be able to iterate through them using Context methods.
    game::interface::BeamContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyString("NAME", "Three");
    a.check("01", testee.next());
    verif.verifyString("NAME", "Five");
    a.check("02", testee.next());
    verif.verifyString("NAME", "Six");
    a.check("03", !testee.next());
}

/** Test behaviour on non-existant object.
    Normally, such a BeamContext instance cannot be created. */
AFL_TEST("game.interface.BeamContext:null", a)
{
    // Given an environment with no beams...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();

    // ...I expect BeamContext to report all properties as null...
    game::interface::BeamContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyNull("NAME");
    verif.verifyNull("TECH");

    // ...and nothing to be assignable.
    AFL_CHECK_THROWS(a("setStringValue"), verif.setStringValue("NAME", "x"), interpreter::Error);
}

/** Test creation using factory function. */
AFL_TEST("game.interface.BeamContext:create", a)
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
        a.checkNonNull("create 3", p.get());
        interpreter::test::ContextVerifier(*p, a).verifyString("NAME", "Three");
    }

    // ...but not for any other Id.
    {
        std::auto_ptr<game::interface::BeamContext> p(game::interface::BeamContext::create(0, session));
        a.checkNull("create 0", p.get());
    }
    {
        std::auto_ptr<game::interface::BeamContext> p(game::interface::BeamContext::create(10, session));
        a.checkNull("create 10", p.get());
    }
}

/** Test set(). */
AFL_TEST("game.interface.BeamContext:set", a)
{
    // Given an environment with a beam...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    shipList->beams().create(3)->setName("Three");

    // ...I expect to be able to change the Name property...
    game::interface::BeamContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, a);
    AFL_CHECK_SUCCEEDS(a("NAME"), verif.setStringValue("NAME", "New"));
    a.checkEqual("getName", shipList->beams().get(3)->getName(shipList->componentNamer()), "New");

    // ...but not the Id or other properties.
    AFL_CHECK_THROWS(a("ID"), verif.setIntegerValue("ID", 8), interpreter::Error);
    AFL_CHECK_THROWS(a("DAMAGE"), verif.setIntegerValue("DAMAGE", 8), interpreter::Error);
}
