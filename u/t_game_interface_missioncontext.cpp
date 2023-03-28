/**
  *  \file u/t_game_interface_missioncontext.cpp
  *  \brief Test for game::interface::MissionContext
  */

#include "game/interface/missioncontext.hpp"

#include "t_game_interface.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"

/** Test basics: general behaviour, specific properties. */
void
TestGameInterfaceMissionContext::testIt()
{
    // Create a ship list
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // Add a mission
    shipList->missions().addMission(game::spec::Mission(8, "!is*,Intercept a ship"));
    TS_ASSERT_EQUALS(shipList->missions().size(), 1U);

    // Test
    game::interface::MissionContext testee(0, shipList);
    interpreter::test::ContextVerifier verif(testee, "testIt");
    verif.verifyTypes();
    verif.verifyBasics();
    verif.verifyNotSerializable();
    TS_ASSERT(testee.getObject() == 0);

    verif.verifyString("NAME", "Intercept a ship");
    verif.verifyInteger("NUMBER", 8);

    // Not assignable
    TS_ASSERT_THROWS(verif.setStringValue("NAME", "New Name"), interpreter::Error);
}

/** Test iteration. */
void
TestGameInterfaceMissionContext::testIteration()
{
    // Create a ship list
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // Add a mission
    shipList->missions().addMission(game::spec::Mission(8, "!is*,Intercept"));
    shipList->missions().addMission(game::spec::Mission(9, "+5,Rob Ship"));
    shipList->missions().addMission(game::spec::Mission(9, "+6,Self Repair"));
    TS_ASSERT_EQUALS(shipList->missions().size(), 3U);

    // Test
    game::interface::MissionContext testee(0, shipList);
    interpreter::test::ContextVerifier verif(testee, "testIteration");
    verif.verifyString("NAME", "Intercept");
    TS_ASSERT(testee.next());
    verif.verifyString("NAME", "Rob Ship");
    TS_ASSERT(testee.next());
    verif.verifyString("NAME", "Self Repair");
    TS_ASSERT(!testee.next());
}

/** Test behaviour on non-existant mission.
    Normally, such a MissionContext instance cannot be created. */
void
TestGameInterfaceMissionContext::testNull()
{
    // Create a ship list, but no missions
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // Test
    game::interface::MissionContext testee(0, shipList);
    interpreter::test::ContextVerifier verif(testee, "testNull");
    verif.verifyNull("NAME");
    verif.verifyNull("NUMBER");
}
