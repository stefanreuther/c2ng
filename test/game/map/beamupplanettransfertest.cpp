/**
  *  \file test/game/map/beamupplanettransfertest.cpp
  *  \brief Test for game::map::BeamUpPlanetTransfer
  */

#include "game/map/beamupplanettransfer.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/simpleturn.hpp"
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"

using game::Element;
using game::config::HostConfiguration;
using game::map::Object;
using game::map::Planet;
using game::map::Ship;

/** Basic functionality/coverage test. */
AFL_TEST("game.map.BeamUpPlanetTransfer:basics", a)
{
    afl::string::NullTranslator tx;
    game::test::SimpleTurn t;

    Planet& pl = t.addPlanet(30, 8, Object::NotPlayable);
    pl.setName("World");

    Ship& sh = t.addShip(15, 2, Object::Playable);
    sh.setName("Float");

    t.config()[HostConfiguration::AllowBeamUpClans].set(0);

    game::map::BeamUpPlanetTransfer testee(pl, sh, t.turn(), t.config());

    /*
     *  Basic validation
     */

    a.checkEqual("01. getName", testee.getName(tx), "Beam up from World");
    a.checkEqual("02. getInfo1", testee.getInfo1(tx), "");
    a.checkEqual("03. getInfo2", testee.getInfo2(tx), "");
    a.checkEqual("04. can Neutronium", testee.canHaveElement(Element::Neutronium), true);
    a.checkEqual("05. can Colonists", testee.canHaveElement(Element::Colonists), false);
    a.checkEqual("06. can Fighters", testee.canHaveElement(Element::Fighters), false);
    a.check("07. UnloadTarget", testee.getFlags().contains(game::CargoContainer::UnloadTarget));

    /* Planet has 1000 of each, so minimum is -9000 to allow taking 10000 */
    a.checkEqual("11. min Neutronium", testee.getMinAmount(Element::Neutronium), -9000);
    a.checkEqual("12. Neutronium", testee.getAmount(Element::Neutronium), 1000);
    a.check("13. max Neutronium", testee.getMaxAmount(Element::Neutronium) > 10000);
}

/** Test BeamUpPlanetTransfer with a transfer command present. */
AFL_TEST("game.map.BeamUpPlanetTransfer:command", a)
{
    const int PLAYER_ID = 2;

    afl::string::NullTranslator tx;
    game::test::SimpleTurn t;

    // Planet
    Planet& pl = t.addPlanet(30, 8, Object::NotPlayable);
    pl.setName("World");

    // Ship
    Ship& sh = t.addShip(15, PLAYER_ID, Object::Playable);
    sh.setName("Boat");

    // Config
    t.config()[HostConfiguration::AllowBeamUpClans].set(1);

    // Command
    game::v3::CommandExtra::create(t.turn()).create(PLAYER_ID).addCommand(game::v3::Command::BeamUp, 15, "n30");

    // Create object under test
    game::map::BeamUpPlanetTransfer testee(pl, sh, t.turn(), t.config());

    // Verify
    a.checkEqual("01. getName", testee.getName(tx), "Beam up from World");
    a.checkEqual("02. can Neutronium", testee.canHaveElement(Element::Neutronium), true);
    a.checkEqual("03. can Colonists", testee.canHaveElement(Element::Colonists), true);
    a.checkEqual("04. can Fighters", testee.canHaveElement(Element::Fighters), false);
    a.check("05. UnloadTarget", testee.getFlags().contains(game::CargoContainer::UnloadTarget));

    /* Planet has 1000 of each, so minimum is -9000 to allow taking 10000 */
    a.checkEqual("11. min Neutronium", testee.getMinAmount(Element::Neutronium), -9000);
    a.checkEqual("12. Neutronium", testee.getAmount(Element::Neutronium), 970);          // we already beam up 30
    a.check("13. max Neutronium", testee.getMaxAmount(Element::Neutronium) > 10000);
}
