/**
  *  \file test/game/interface/missioncontexttest.cpp
  *  \brief Test for game::interface::MissionContext
  */

#include "game/interface/missioncontext.hpp"

#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"

/** Test basics: general behaviour, specific properties. */
AFL_TEST("game.interface.MissionContext:basics", a)
{
    // Create a ship list
    afl::base::Ref<game::spec::MissionList> list(game::spec::MissionList::create());

    // Add a mission
    list->addMission(game::spec::Mission(8, "!is*,Intercept a ship"));
    a.checkEqual("01. size", list->size(), 1U);

    // Test
    game::interface::MissionContext testee(0, list);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyTypes();
    verif.verifyBasics();
    verif.verifyNotSerializable();
    a.checkNull("11. getObject", testee.getObject());

    verif.verifyString("NAME", "Intercept a ship");
    verif.verifyInteger("NUMBER", 8);

    // Not assignable
    AFL_CHECK_THROWS(a("21. set NAME"), verif.setStringValue("NAME", "New Name"), interpreter::Error);

    // getMission
    const game::spec::Mission* msn = testee.getMission();
    a.checkNonNull("31. getMission", msn);
    a.checkEqual("32. getNumber", msn->getNumber(), 8);
}

/** Test iteration. */
AFL_TEST("game.interface.MissionContext:iteration", a)
{
    // Create a ship list
    afl::base::Ref<game::spec::MissionList> list(game::spec::MissionList::create());

    // Add a mission
    list->addMission(game::spec::Mission(8, "!is*,Intercept"));
    list->addMission(game::spec::Mission(9, "+5,Rob Ship"));
    list->addMission(game::spec::Mission(9, "+6,Self Repair"));
    a.checkEqual("01. size", list->size(), 3U);

    // Test
    game::interface::MissionContext testee(0, list);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyString("NAME", "Intercept");
    a.check("11. next", testee.next());
    verif.verifyString("NAME", "Rob Ship");
    a.check("12. next", testee.next());
    verif.verifyString("NAME", "Self Repair");
    a.check("13. next", !testee.next());
}

/** Test behaviour on non-existant mission.
    Normally, such a MissionContext instance cannot be created. */
AFL_TEST("game.interface.MissionContext:null", a)
{
    // Create a ship list, but no missions
    afl::base::Ref<game::spec::MissionList> list(game::spec::MissionList::create());

    // Test
    game::interface::MissionContext testee(0, list);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyNull("NAME");
    verif.verifyNull("NUMBER");
}
