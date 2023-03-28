/**
  *  \file u/t_game_interface_basetaskbuildcommandparser.cpp
  *  \brief Test for game::interface::BaseTaskBuildCommandParser
  */

#include "game/interface/basetaskbuildcommandparser.hpp"

#include "t_game_interface.hpp"
#include "game/spec/shiplist.hpp"

void
TestGameInterfaceBaseTaskBuildCommandParser::testIt()
{
    // Populate a ship list
    game::spec::ShipList shipList;
    for (int i = 1; i <= 5; ++i) {
        shipList.launchers().create(i);
    }
    for (int i = 1; i <= 7; ++i) {
        shipList.beams().create(i);
    }
    for (int i = 1; i <= 9; ++i) {
        shipList.engines().create(i);
    }
    for (int i = 1; i <= 11; ++i) {
        game::spec::Hull* h = shipList.hulls().create(i);
        h->setMaxLaunchers(4);
        h->setMaxBeams(6);
    }

    // Test: BuildShip command
    {
        game::interface::BaseTaskBuildCommandParser p(shipList);
        p.predictStatement("BuildShip 10, 5, 3, -1, 2, -1");
        TS_ASSERT_EQUALS(p.getVerb(), "BUILDSHIP");
        TS_ASSERT_EQUALS(p.getOrder().getHullIndex(), 10);
        TS_ASSERT_EQUALS(p.getOrder().getEngineType(), 5);
        TS_ASSERT_EQUALS(p.getOrder().getBeamType(), 3);
        TS_ASSERT_EQUALS(p.getOrder().getNumBeams(), 6);
        TS_ASSERT_EQUALS(p.getOrder().getTorpedoType(), 2);
        TS_ASSERT_EQUALS(p.getOrder().getNumLaunchers(), 4);
    }

    // Test: EnqueueShip command
    {
        game::interface::BaseTaskBuildCommandParser p(shipList);
        p.predictStatement("EnqueueShip 5, 2");
        TS_ASSERT_EQUALS(p.getVerb(), "ENQUEUESHIP");
        TS_ASSERT_EQUALS(p.getOrder().getHullIndex(), 5);
        TS_ASSERT_EQUALS(p.getOrder().getEngineType(), 2);
        TS_ASSERT_EQUALS(p.getOrder().getBeamType(), 0);
        TS_ASSERT_EQUALS(p.getOrder().getNumBeams(), 0);
        TS_ASSERT_EQUALS(p.getOrder().getTorpedoType(), 0);
        TS_ASSERT_EQUALS(p.getOrder().getNumLaunchers(), 0);
    }

    // Test: other command
    {
        game::interface::BaseTaskBuildCommandParser p(shipList);
        p.predictStatement("GotoShip 5, 2");
        TS_ASSERT_EQUALS(p.getVerb(), "");
    }

    // Test: cancel build order
    {
        game::interface::BaseTaskBuildCommandParser p(shipList);
        p.predictStatement("BuildShip 0");
        TS_ASSERT_EQUALS(p.getVerb(), "BUILDSHIP");
        TS_ASSERT_EQUALS(p.getOrder().getHullIndex(), 0);
    }

    // Test: hull out of range
    //   Exceptions are swallowed by predictStatement().
    //   We therefore just do not recognize the command.
    {
        game::interface::BaseTaskBuildCommandParser p(shipList);
        p.predictStatement("BuildShip 12");
        TS_ASSERT_EQUALS(p.getVerb(), "");
    }

    // Test: engine out of range
    {
        game::interface::BaseTaskBuildCommandParser p(shipList);
        p.predictStatement("BuildShip 5, 0");
        TS_ASSERT_EQUALS(p.getVerb(), "");
    }

    // Test: arity error
    {
        game::interface::BaseTaskBuildCommandParser p(shipList);
        p.predictStatement("BuildShip");
        TS_ASSERT_EQUALS(p.getVerb(), "");
    }
}

