/**
  *  \file u/t_game_v3_udata_sessionnameprovider.cpp
  *  \brief Test for game::v3::udata::SessionNameProvider
  */

#include "game/v3/udata/sessionnameprovider.hpp"

#include "t_game_v3_udata.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"

using game::v3::udata::NameProvider;

/** Test behaviour on empty session. */
void
TestGameV3UdataSessionNameProvider::testEmpty()
{
    // Make empty session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);


    // Test
    game::v3::udata::SessionNameProvider testee(session);
    TS_ASSERT_EQUALS(testee.getName(NameProvider::HullFunctionName, 10),    "");
    TS_ASSERT_EQUALS(testee.getName(NameProvider::HullName, 10),            "");
    TS_ASSERT_EQUALS(testee.getName(NameProvider::NativeGovernmentName, 5), "Feudal");
    TS_ASSERT_EQUALS(testee.getName(NameProvider::NativeRaceName, 2),       "Bovinoid");
    TS_ASSERT_EQUALS(testee.getName(NameProvider::PlanetName, 10),          "");
    TS_ASSERT_EQUALS(testee.getName(NameProvider::ShortRaceName, 10),       "");
}

/** Test behaviour on populated session (normal case). */
void
TestGameV3UdataSessionNameProvider::testPopulated()
{
    // Make empty session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Create empty objects
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    session.setShipList(new game::spec::ShipList());

    // Populate
    session.getShipList()->basicHullFunctions().addFunction(10, "SporeDrive");
    session.getShipList()->hulls().create(10)->setName("Olympic Class");
    session.getRoot()->playerList().create(10)->setName(game::Player::ShortName, "The Rebels");
    session.getGame()->currentTurn().universe().planets().create(10)->setName("Vulcan");

    // Test
    game::v3::udata::SessionNameProvider testee(session);
    TS_ASSERT_EQUALS(testee.getName(NameProvider::HullFunctionName, 10),    "SporeDrive");
    TS_ASSERT_EQUALS(testee.getName(NameProvider::HullName, 10),            "Olympic Class");
    TS_ASSERT_EQUALS(testee.getName(NameProvider::NativeGovernmentName, 5), "Feudal");
    TS_ASSERT_EQUALS(testee.getName(NameProvider::NativeRaceName, 2),       "Bovinoid");
    TS_ASSERT_EQUALS(testee.getName(NameProvider::PlanetName, 10),          "Vulcan");
    TS_ASSERT_EQUALS(testee.getName(NameProvider::ShortRaceName, 10),       "The Rebels");
}

