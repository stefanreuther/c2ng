/**
  *  \file u/t_game_map_beamupshiptransfer.cpp
  *  \brief Test for game::map::BeamupShipTransfer
  */

#include "game/map/beamupshiptransfer.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/simpleturn.hpp"
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"

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
    sh.setName("Scotty");
    afl::string::NullTranslator tx;

    game::map::BeamUpShipTransfer testee(sh, h.shipList(), h.turn(), h.config());

    /*
     *  Ship has a fuel tank of 100 with 10N (=100 max).
     *  Ship has a cargo bay of 100 with 10T, 10D, 10M, 10S, 10C (=60 max of each).
     */

    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Neutronium), 100);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Tritanium), 60);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Duranium), 60);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Money), 10000);
    TS_ASSERT_EQUALS(testee.getName(tx), "Scotty");

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

    const Command* cmd = cc->getCommand(Command::BeamUp, SHIP_ID);
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

    CommandExtra::create(h.turn()).create(SHIP_OWNER).addCommand(Command::BeamUp, SHIP_ID, "C30 M10");

    game::map::BeamUpShipTransfer testee(sh, h.shipList(), h.turn(), h.config());

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

/** Test behaviour or BeamUpShipTransfer with a command present; test removal of command. */
void
TestGameMapBeamUpShipTransfer::testCommand()
{
    const int SHIP_ID = 10;
    const int SHIP_OWNER = 6;

    // Environment/Ship
    SimpleTurn h;
    Ship& sh = h.addShip(SHIP_ID, SHIP_OWNER, Object::Playable);
    sh.setName("Scotty");
    sh.setCargo(Element::Neutronium, 10);
    sh.setMission(35, 0, 0);                         // default Beam Up Multi
    afl::string::NullTranslator tx;
    h.config()[game::config::HostConfiguration::AllowBeamUpClans].set(0);

    // Command
    CommandContainer& cc = CommandExtra::create(h.turn()).create(SHIP_OWNER);
    cc.addCommand(Command::BeamUp, SHIP_ID, "n30");

    // Testee
    game::map::BeamUpShipTransfer testee(sh, h.shipList(), h.turn(), h.config());

    // Verify
    TS_ASSERT(!testee.getFlags().contains(game::CargoContainer::UnloadTarget));
    TS_ASSERT_EQUALS(testee.canHaveElement(Element::Neutronium), true);
    TS_ASSERT_EQUALS(testee.canHaveElement(Element::Colonists), false);
    TS_ASSERT_EQUALS(testee.getAmount(Element::Neutronium), 40);           // 10 on ship + 30 beaming up
    TS_ASSERT_EQUALS(testee.getMinAmount(Element::Neutronium), 10);

    // Unload
    testee.change(Element::Neutronium, -30);
    testee.commit();

    // Verify
    TS_ASSERT(cc.getCommand(Command::BeamUp, SHIP_ID) == 0);
    TS_ASSERT_EQUALS(sh.getMission().orElse(-1), 0);
}

