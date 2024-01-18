/**
  *  \file test/game/interface/hullcontexttest.cpp
  *  \brief Test for game::interface::HullContext
  */

#include "game/interface/hullcontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "interpreter/test/contextverifier.hpp"

using game::spec::Cost;

/** Test basics: general behaviour, specific properties. */
AFL_TEST("game.interface.HullContext:basics", a)
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
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifySerializable(interpreter::TagNode::Tag_Hull, 3, afl::base::Nothing);
    verif.verifyTypes();
    a.checkNull("01. getObject)", testee.getObject());

    // Specific properties
    a.checkEqual("11. toString", testee.toString(true), "Hull(3)");
    verif.verifyInteger("TECH", 9);
    verif.verifyInteger("COST.D", 7);
    verif.verifyString("NAME", "Orville");
    verif.verifyString("SPECIAL", "");
}

/** Test iteration. */
AFL_TEST("game.interface.HullContext:iteration", a)
{
    // Given an environment with multiple hulls...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    shipList->hulls().create(3)->setName("Three");
    shipList->hulls().create(5)->setName("Five");
    shipList->hulls().create(6)->setName("Six");

    // ...I expect to be able to iterate through them using Context methods.
    game::interface::HullContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyString("NAME", "Three");
    a.check("01. next", testee.next());
    verif.verifyString("NAME", "Five");
    a.check("02. next", testee.next());
    verif.verifyString("NAME", "Six");
    a.check("03. next", !testee.next());
}

/** Test behaviour on non-existant object.
    Normally, such a HullContext instance cannot be created. */
AFL_TEST("game.interface.HullContext:null", a)
{
    // Given an environment with no hulls...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();

    // ...I expect HullContext to report all properties as null...
    game::interface::HullContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyNull("NAME");
    verif.verifyNull("TECH");

    // ...and nothing to be assignable.
    AFL_CHECK_THROWS(a, verif.setStringValue("NAME", "x"), interpreter::Error);
}

/** Test creation using factory function. */
AFL_TEST("game.interface.HullContext:create", a)
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
        a.checkNonNull("01. create", p.get());
        interpreter::test::ContextVerifier(*p, a("02. create")).verifyString("NAME", "Three");
    }

    // ...but not for any other Id.
    {
        std::auto_ptr<game::interface::HullContext> p(game::interface::HullContext::create(0, session));
        a.checkNull("11. create 0", p.get());
    }
    {
        std::auto_ptr<game::interface::HullContext> p(game::interface::HullContext::create(10, session));
        a.checkNull("12. create 10", p.get());
    }
}

/** Test set(). */
AFL_TEST("game.interface.HullContext:set", a)
{
    // Given an environment with a hull...
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList = *new game::spec::ShipList();
    shipList->hulls().create(3)->setName("Three");

    // ...I expect to be able to change the Name property...
    game::interface::HullContext testee(3, shipList, root);
    interpreter::test::ContextVerifier verif(testee, a);
    AFL_CHECK_SUCCEEDS(a("01. set NAME"), verif.setStringValue("NAME", "New"));
    AFL_CHECK_SUCCEEDS(a("02. set IMAGE"), verif.setIntegerValue("IMAGE", 555));
    a.checkEqual("03. getName", shipList->hulls().get(3)->getName(shipList->componentNamer()), "New");
    a.checkEqual("04. getInternalPictureNumber", shipList->hulls().get(3)->getInternalPictureNumber(), 555);

    // ...but not the Id or other properties.
    AFL_CHECK_THROWS(a("11. set ID"), verif.setIntegerValue("ID", 8), interpreter::Error);
    AFL_CHECK_THROWS(a("12. set TECH"), verif.setIntegerValue("TECH", 8), interpreter::Error);
}
