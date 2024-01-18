/**
  *  \file test/game/map/beamupshiptransfertest.cpp
  *  \brief Test for game::map::BeamUpShipTransfer
  */

#include "game/map/beamupshiptransfer.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.map.BeamUpShipTransfer:basics", a)
{
    const int SHIP_ID = 10;
    const int SHIP_OWNER = 6;

    SimpleTurn h;
    Ship& sh = h.addShip(SHIP_ID, SHIP_OWNER, Object::Playable);
    sh.setName("Scotty");
    afl::string::NullTranslator tx;

    game::map::BeamUpShipTransfer testee(sh, h.shipList(), h.turn(), h.mapConfiguration(), h.config());

    /*
     *  Ship has a fuel tank of 100 with 10N (=100 max).
     *  Ship has a cargo bay of 100 with 10T, 10D, 10M, 10S, 10C (=60 max of each).
     */

    a.checkEqual("01. max Neutronium", testee.getMaxAmount(Element::Neutronium), 100);
    a.checkEqual("02. max Tritanium",  testee.getMaxAmount(Element::Tritanium), 60);
    a.checkEqual("03. max Duranium",   testee.getMaxAmount(Element::Duranium), 60);
    a.checkEqual("04. max Money",      testee.getMaxAmount(Element::Money), 10000);
    a.checkEqual("05. getName",        testee.getName(tx), "Scotty");
    a.checkEqual("06. getInfo1",       testee.getInfo1(tx), "");
    a.checkEqual("07. getInfo2",       testee.getInfo2(tx), "");

    // Add some cargo
    testee.change(Element::Tritanium, 20);
    a.checkEqual("11. max Tritanium",  testee.getMaxAmount(Element::Tritanium), 60);   // unchanged
    a.checkEqual("12. max Duranium",   testee.getMaxAmount(Element::Duranium), 40);    // -20

    testee.change(Element::Neutronium, 15);
    a.checkEqual("21. max Neutronium", testee.getMaxAmount(Element::Neutronium), 100); // unchanged
    a.checkEqual("22. max Tritanium",  testee.getMaxAmount(Element::Tritanium), 60);   // unchanged
    a.checkEqual("23. max Duranium",   testee.getMaxAmount(Element::Duranium), 40);    // unchanged

    // Commit
    testee.commit();

    // Ship content is unchanged
    a.checkEqual("31. Neutronium", sh.getCargo(Element::Neutronium).orElse(0), 10);
    a.checkEqual("32. Tritanium",  sh.getCargo(Element::Tritanium).orElse(0), 10);
    a.checkEqual("33. Duranium",   sh.getCargo(Element::Duranium).orElse(0), 10);
    a.checkEqual("34. Molybdenum", sh.getCargo(Element::Molybdenum).orElse(0), 10);

    // BeamUpShipTransfer creates the command
    CommandContainer* cc = CommandExtra::get(h.turn(), SHIP_OWNER);
    a.checkNonNull("41. cc", cc);

    const Command* cmd = cc->getCommand(Command::BeamUp, SHIP_ID);
    a.checkNonNull("51. cmd", cmd);
    a.checkEqual("52. getArg", cmd->getArg(), "N15 T20");

}

/** Test command parsing. */
AFL_TEST("game.map.BeamUpShipTransfer:parse", a)
{
    const int SHIP_ID = 10;
    const int SHIP_OWNER = 6;

    SimpleTurn h;
    Ship& sh = h.addShip(SHIP_ID, SHIP_OWNER, Object::Playable);

    CommandExtra::create(h.turn()).create(SHIP_OWNER).addCommand(Command::BeamUp, SHIP_ID, "C30 M10");

    game::map::BeamUpShipTransfer testee(sh, h.shipList(), h.turn(), h.mapConfiguration(), h.config());

    // Initial changes still zero
    a.checkEqual("01. Colonists change",  testee.getChange(Element::Colonists), 0);
    a.checkEqual("02. Neutronium change", testee.getChange(Element::Neutronium), 0);
    a.checkEqual("03. Molybdenum change", testee.getChange(Element::Molybdenum), 0);

    // Changes included in effective amount
    a.checkEqual("11. Colonists",         testee.getAmount(Element::Colonists), 40);
    a.checkEqual("12. Neutronium",        testee.getAmount(Element::Neutronium), 10);
    a.checkEqual("13. Molybdenum",        testee.getAmount(Element::Molybdenum), 20);

    // Effective content
    a.checkEqual("21. max Neutronium",    testee.getMaxAmount(Element::Neutronium), 100);
    a.checkEqual("22. max Tritanium",     testee.getMaxAmount(Element::Tritanium), 20);
    a.checkEqual("23. max Duranium",      testee.getMaxAmount(Element::Duranium), 20);
    a.checkEqual("24. max Molybdenum",    testee.getMaxAmount(Element::Molybdenum), 30);
    a.checkEqual("25. max Colonists",     testee.getMaxAmount(Element::Colonists), 50);
    a.checkEqual("26. max Money",         testee.getMaxAmount(Element::Money), 10000);
}

/** Test behaviour or BeamUpShipTransfer with a command present; test removal of command. */
AFL_TEST("game.map.BeamUpShipTransfer:command", a)
{
    const int SHIP_ID = 10;
    const int SHIP_OWNER = 6;

    // Environment/Ship
    SimpleTurn h;
    Ship& sh = h.addShip(SHIP_ID, SHIP_OWNER, Object::Playable);
    sh.setName("Scotty");
    sh.setCargo(Element::Neutronium, 10);
    sh.setMission(35, 0, 0);                         // default Beam Up Multi
    h.config()[game::config::HostConfiguration::AllowBeamUpClans].set(0);

    // Command
    CommandContainer& cc = CommandExtra::create(h.turn()).create(SHIP_OWNER);
    cc.addCommand(Command::BeamUp, SHIP_ID, "n30");

    // Testee
    game::map::BeamUpShipTransfer testee(sh, h.shipList(), h.turn(), h.mapConfiguration(), h.config());

    // Verify
    a.check("01. UnloadTarget", !testee.getFlags().contains(game::CargoContainer::UnloadTarget));
    a.checkEqual("02. can Neutronium", testee.canHaveElement(Element::Neutronium), true);
    a.checkEqual("03. can Colonists",  testee.canHaveElement(Element::Colonists), false);
    a.checkEqual("04. Neutronium",     testee.getAmount(Element::Neutronium), 40);           // 10 on ship + 30 beaming up
    a.checkEqual("05. min Neutronium", testee.getMinAmount(Element::Neutronium), 10);

    // Unload
    testee.change(Element::Neutronium, -30);
    testee.commit();

    // Verify
    a.checkNull("11. getCommand", cc.getCommand(Command::BeamUp, SHIP_ID));
    a.checkEqual("12. getMission", sh.getMission().orElse(-1), 0);
}
