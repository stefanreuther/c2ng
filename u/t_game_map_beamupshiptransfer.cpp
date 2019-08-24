/**
  *  \file u/t_game_map_beamupshiptransfer.cpp
  *  \brief Test for game::map::BeamupShipTransfer
  */

#include "game/map/beamupshiptransfer.hpp"

#include "t_game_map.hpp"
#include "game/test/simpleturn.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"
#include "game/v3/command.hpp"

using game::Element;
using game::map::Object;
using game::map::Ship;
using game::test::SimpleTurn;
using game::v3::Command;
using game::v3::CommandContainer;
using game::v3::CommandExtra;

/** Simple test: do a transfer, validate parameters and result creation. */
void
TestGameMapBeamupShipTransfer::testIt()
{
    const int SHIP_ID = 10;
    const int SHIP_OWNER = 6;

    SimpleTurn h;
    Ship& sh = h.addShip(SHIP_ID, SHIP_OWNER, Object::Playable);

    game::map::BeamUpShipTransfer testee(sh, h.interface(), h.shipList(), h.turn(), h.config());

    /*
     *  Ship has a fuel tank of 100 with 10N (=100 max).
     *  Ship has a cargo bay of 100 with 10T, 10D, 10M, 10S, 10C (=60 max of each).
     */

    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Neutronium), 100);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Tritanium), 60);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Duranium), 60);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Money), 10000);

    // Add some cargo
    testee.change(Element::Tritanium, 20);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Tritanium), 60);   // unchanged
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Duranium), 40);    // -20

    testee.change(Element::Neutronium, 15);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Neutronium), 100); // unchanged
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Tritanium), 60);   // unchanged
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Duranium), 40);    // unchanged

    // Commit
    testee.commit();

    // Ship content is unchanged
    TS_ASSERT_EQUALS(sh.getCargo(Element::Neutronium).orElse(0), 10);
    TS_ASSERT_EQUALS(sh.getCargo(Element::Tritanium).orElse(0), 10);
    TS_ASSERT_EQUALS(sh.getCargo(Element::Duranium).orElse(0), 10);
    TS_ASSERT_EQUALS(sh.getCargo(Element::Molybdenum).orElse(0), 10);

    // BeamUpShipTransfer creates the command
    CommandContainer* cc = CommandExtra::get(h.turn(), SHIP_OWNER);
    TS_ASSERT(cc != 0);

    const Command* cmd = cc->getCommand(Command::phc_Beamup, SHIP_ID);
    TS_ASSERT(cmd != 0);
    TS_ASSERT_EQUALS(cmd->getArg(), "N15 T20");
    
}

/** Test command parsing. */
void
TestGameMapBeamUpShipTransfer::testParse()
{
    const int SHIP_ID = 10;
    const int SHIP_OWNER = 6;

    SimpleTurn h;
    Ship& sh = h.addShip(SHIP_ID, SHIP_OWNER, Object::Playable);

    CommandExtra::create(h.turn()).create(SHIP_OWNER).addCommand(Command::phc_Beamup, SHIP_ID, "C30 M10");

    game::map::BeamUpShipTransfer testee(sh, h.interface(), h.shipList(), h.turn(), h.config());

    // Initial changes still zero
    TS_ASSERT_EQUALS(testee.getChange(Element::Colonists), 0);
    TS_ASSERT_EQUALS(testee.getChange(Element::Neutronium), 0);
    TS_ASSERT_EQUALS(testee.getChange(Element::Molybdenum), 0);

    // Changes included in effective amount
    TS_ASSERT_EQUALS(testee.getAmount(Element::Colonists), 40);
    TS_ASSERT_EQUALS(testee.getAmount(Element::Neutronium), 10);
    TS_ASSERT_EQUALS(testee.getAmount(Element::Molybdenum), 20);

    // Effective content
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Neutronium), 100);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Tritanium), 20);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Duranium), 20);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Molybdenum), 30);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Colonists), 50);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Money), 10000);
}

