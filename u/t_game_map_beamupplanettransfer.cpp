/**
  *  \file u/t_game_map_beamupplanettransfer.cpp
  *  \brief Test for game::map::BeamUpPlanetTransfer
  */

#include "game/map/beamupplanettransfer.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
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
void
TestGameMapBeamUpPlanetTransfer::testIt()
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

    TS_ASSERT_EQUALS(testee.getName(tx), "Beam up from World");
    TS_ASSERT_EQUALS(testee.getInfo1(tx), "");
    TS_ASSERT_EQUALS(testee.getInfo2(tx), "");
    TS_ASSERT_EQUALS(testee.canHaveElement(Element::Neutronium), true);
    TS_ASSERT_EQUALS(testee.canHaveElement(Element::Colonists), false);
    TS_ASSERT_EQUALS(testee.canHaveElement(Element::Fighters), false);
    TS_ASSERT(testee.getFlags().contains(game::CargoContainer::UnloadTarget));

    /* Planet has 1000 of each, so minimum is -9000 to allow taking 10000 */
    TS_ASSERT_EQUALS(testee.getMinAmount(Element::Neutronium), -9000);
    TS_ASSERT_EQUALS(testee.getAmount(Element::Neutronium), 1000);
    TS_ASSERT(testee.getMaxAmount(Element::Neutronium) > 10000);
}

/** Test BeamUpPlanetTransfer with a transfer command present. */
void
TestGameMapBeamUpPlanetTransfer::testCommand()
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
    TS_ASSERT_EQUALS(testee.getName(tx), "Beam up from World");
    TS_ASSERT_EQUALS(testee.canHaveElement(Element::Neutronium), true);
    TS_ASSERT_EQUALS(testee.canHaveElement(Element::Colonists), true);
    TS_ASSERT_EQUALS(testee.canHaveElement(Element::Fighters), false);
    TS_ASSERT(testee.getFlags().contains(game::CargoContainer::UnloadTarget));

    /* Planet has 1000 of each, so minimum is -9000 to allow taking 10000 */
    TS_ASSERT_EQUALS(testee.getMinAmount(Element::Neutronium), -9000);
    TS_ASSERT_EQUALS(testee.getAmount(Element::Neutronium), 970);          // we already beam up 30
    TS_ASSERT(testee.getMaxAmount(Element::Neutronium) > 10000);
}

