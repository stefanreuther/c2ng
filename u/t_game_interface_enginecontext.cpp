/**
  *  \file u/t_game_interface_enginecontext.cpp
  *  \brief Test for game::interface::EngineContext
  */

#include "game/interface/enginecontext.hpp"

#include "t_game_interface.hpp"
#include "u/helper/contextverifier.hpp"

/** Test engine. */
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

    // Verify
    game::interface::EngineContext ctx(NR, shipList);
    verifyTypes(ctx);

    // Verify individual properties
    verifyInteger(ctx, "ID", NR);
    verifyInteger(ctx, "TECH", 3);
    verifyString(ctx, "NAME", "The Kettle");
}

