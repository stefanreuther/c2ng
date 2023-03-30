/**
  *  \file u/t_game_interface_missioncontext.cpp
  *  \brief Test for game::interface::MissionContext
  */

#include "game/interface/missioncontext.hpp"

#include "t_game_interface.hpp"
#include "interpreter/test/contextverifier.hpp"

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
}

