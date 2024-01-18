/**
  *  \file test/game/v3/udata/sessionnameprovidertest.cpp
  *  \brief Test for game::v3::udata::SessionNameProvider
  */

#include "game/v3/udata/sessionnameprovider.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"

using game::v3::udata::NameProvider;

/** Test behaviour on empty session. */
AFL_TEST("game.v3.udata.SessionNameProvider:empty", a)
{
    // Make empty session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);


    // Test
    game::v3::udata::SessionNameProvider testee(session);
    a.checkEqual("01. HullFunctionName",     testee.getName(NameProvider::HullFunctionName, 10),    "");
    a.checkEqual("02. HullName",             testee.getName(NameProvider::HullName, 10),            "");
    a.checkEqual("03. NativeGovernmentName", testee.getName(NameProvider::NativeGovernmentName, 5), "Feudal");
    a.checkEqual("04. NativeRaceName",       testee.getName(NameProvider::NativeRaceName, 2),       "Bovinoid");
    a.checkEqual("05. PlanetName",           testee.getName(NameProvider::PlanetName, 10),          "");
    a.checkEqual("06. ShortRaceName",        testee.getName(NameProvider::ShortRaceName, 10),       "");
}

/** Test behaviour on populated session (normal case). */
AFL_TEST("game.v3.udata.SessionNameProvider:normal", a)
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
    a.checkEqual("01. HullFunctionName",     testee.getName(NameProvider::HullFunctionName, 10),    "SporeDrive");
    a.checkEqual("02. HullName",             testee.getName(NameProvider::HullName, 10),            "Olympic Class");
    a.checkEqual("03. NativeGovernmentName", testee.getName(NameProvider::NativeGovernmentName, 5), "Feudal");
    a.checkEqual("04. NativeRaceName",       testee.getName(NameProvider::NativeRaceName, 2),       "Bovinoid");
    a.checkEqual("05. PlanetName",           testee.getName(NameProvider::PlanetName, 10),          "Vulcan");
    a.checkEqual("06. ShortRaceName",        testee.getName(NameProvider::ShortRaceName, 10),       "The Rebels");
}
