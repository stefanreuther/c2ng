/**
  *  \file test/game/nu/loadertest.cpp
  *  \brief Test for game::nu::Loader
  */

#include "game/nu/loader.hpp"

#include "afl/data/access.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/value.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/map/planet.hpp"
#include "game/player.hpp"
#include "game/spec/cost.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "game/vcr/battle.hpp"
#include "game/vcr/database.hpp"
#include "game/vcr/object.hpp"
#include "util/io.hpp"

using game::Element;
using game::Player;
using game::PlayerSet_t;
using game::config::HostConfiguration;
using game::spec::AdvantageList;
using game::spec::BasicHullFunction;
using game::spec::Cost;

namespace {
    game::PlayerSet_t getPlayersThatCan(const game::spec::Hull& hull,
                                        const game::spec::ShipList& shipList,
                                        const game::Root& root,
                                        int basicFunctionId)
    {
        return hull.getHullFunctions(true)
            .getPlayersThatCan(basicFunctionId, shipList.modifiedHullFunctions(), shipList.basicHullFunctions(), root.hostConfiguration(),
                               hull, game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS), true);
    }
}

/* Coarse general test for ship list */
AFL_TEST("game.nu.Loader:loadShipList", a)
{
    /* Specification file part of an actual result file, ca. 2020, heavily shortened. */
    const char*const SPEC_FILE =
        "{"
        "  \"success\": true,"
        "  \"rst\": {"
        "    \"settings\": {"
        "      \"name\": \"Test Game\","
        "      \"turn\": 40,"
        "      \"shiplimit\": 400,"
        "      \"shipscanrange\": 300"
        "    },"
        "    \"game\": {"
        "      \"name\": \"Test Game\""
        "    },"
        "    \"player\": {"
        "      \"status\": 1,"
        "      \"statusturn\": 1,"
        "      \"accountid\": 3333,"
        "      \"username\": \"ee-player\","
        "      \"email\": \"\","
        "      \"raceid\": 8,"
        "      \"activehulls\": \"1,15,71,\","
        "      \"activeadvantages\": \"5,22,\","
        "      \"id\": 8"
        "    },"
        "    \"players\": ["
        "      {"
        "        \"status\": 1,"
        "        \"accountid\": 1111,"
        "        \"username\": \"fed-player\","
        "        \"email\": \"\","
        "        \"raceid\": 1,"
        "        \"id\": 1"
        "      },"
        "      {"
        "        \"status\": 1,"
        "        \"accountid\": 2222,"
        "        \"username\": \"lizard-player\","
        "        \"email\": \"\","
        "        \"raceid\": 2,"
        "        \"id\": 2"
        "      },"
        "      {"
        "        \"status\": 1,"
        "        \"accountid\": 3333,"
        "        \"username\": \"ee-player\","
        "        \"email\": \"\","
        "        \"raceid\": 8,"
        "        \"activehulls\": \"1,15,71,\","
        "        \"activeadvantages\": \"22,23,46,49,57,77,54,51,55,79,\","
        "        \"id\": 8"
        "      }"
        "    ],"
        "    \"races\": ["
        "      {"
        "        \"name\": \"Unknown\","
        "        \"shortname\": \"Unknown\","
        "        \"adjective\": \"Unknown\","
        "        \"baseadvantages\": \"\","
        "        \"advantages\": \"\","
        "        \"basehulls\": \"\","
        "        \"hulls\": \"\","
        "        \"id\": 0"
        "      },"
        "      {"
        "        \"name\": \"The Solar Federation\","
        "        \"shortname\": \"The Feds\","
        "        \"adjective\": \"Fed\","
        "        \"baseadvantages\": \"1,2,3,4,32,48,49,51,79\","
        "        \"advantages\": \"1,\","
        "        \"basehulls\": \"1,\","
        "        \"hulls\": \"1\","
        "        \"id\": 1"
        "      },"
        "      {"
        "        \"name\": \"The Lizard Alliance\","
        "        \"shortname\": \"The Lizards\","
        "        \"adjective\": \"Lizard\","
        "        \"baseadvantages\": \"5,6,\","
        "        \"advantages\": \"5,22,\","
        "        \"basehulls\": \"15\","
        "        \"hulls\": \"15\","
        "        \"id\": 2"
        "      },"
        "      {"
        "        \"name\": \"The Evil Empire\","
        "        \"shortname\": \"The Evil Empire\","
        "        \"adjective\": \"Empire\","
        "        \"baseadvantages\": \"22,79\","
        "        \"advantages\": \"22\","
        "        \"basehulls\": \"1,15,\","
        "        \"hulls\": \"15,71\","
        "        \"id\": 8"
        "      }"
        "    ],"
        "    \"hulls\": ["
        "      {"
        "        \"name\": \"Outrider Class Scout\","
        "        \"tritanium\": 40,"
        "        \"duranium\": 20,"
        "        \"molybdenum\": 5,"
        "        \"fueltank\": 260,"
        "        \"crew\": 180,"
        "        \"engines\": 1,"
        "        \"mass\": 75,"
        "        \"techlevel\": 1,"
        "        \"cargo\": 40,"
        "        \"fighterbays\": 0,"
        "        \"launchers\": 0,"
        "        \"beams\": 1,"
        "        \"cancloak\": false,"
        "        \"cost\": 50,"
        "        \"special\": \"\","
        "        \"description\": \"\","
        "        \"advantage\": 0,"
        "        \"isbase\": true,"
        "        \"dur\": 0,"
        "        \"tri\": 0,"
        "        \"mol\": 0,"
        "        \"mc\": 0,"
        "        \"parentid\": 0,"
        "        \"academy\": true,"
        "        \"id\": 1"
        "      },"
        "      {"
        "        \"name\": \"Small Deep Space Freighter\","
        "        \"tritanium\": 2,"
        "        \"duranium\": 2,"
        "        \"molybdenum\": 3,"
        "        \"fueltank\": 200,"
        "        \"crew\": 2,"
        "        \"engines\": 1,"
        "        \"mass\": 30,"
        "        \"techlevel\": 1,"
        "        \"cargo\": 70,"
        "        \"fighterbays\": 0,"
        "        \"launchers\": 0,"
        "        \"beams\": 0,"
        "        \"cancloak\": false,"
        "        \"cost\": 10,"
        "        \"special\": \"\","
        "        \"description\": \"\","
        "        \"advantage\": 0,"
        "        \"isbase\": true,"
        "        \"dur\": 0,"
        "        \"tri\": 0,"
        "        \"mol\": 0,"
        "        \"mc\": 0,"
        "        \"parentid\": 0,"
        "        \"academy\": true,"
        "        \"id\": 15"
        "      },"
        "      {"
        "        \"name\": \"Ru25 Gunboat\","
        "        \"tritanium\": 27,"
        "        \"duranium\": 12,"
        "        \"molybdenum\": 25,"
        "        \"fueltank\": 90,"
        "        \"crew\": 10,"
        "        \"engines\": 1,"
        "        \"mass\": 65,"
        "        \"techlevel\": 1,"
        "        \"cargo\": 1,"
        "        \"fighterbays\": 0,"
        "        \"launchers\": 0,"
        "        \"beams\": 4,"
        "        \"cancloak\": false,"
        "        \"cost\": 60,"
        "        \"special\": \"\","
        "        \"description\": \"\","
        "        \"advantage\": 0,"
        "        \"isbase\": true,"
        "        \"dur\": 0,"
        "        \"tri\": 0,"
        "        \"mol\": 0,"
        "        \"mc\": 0,"
        "        \"parentid\": 0,"
        "        \"academy\": false,"
        "        \"id\": 71"
        "      }"
        "    ],"
        "    \"racehulls\": ["
        "      71,"
        "      15"
        "    ],"
        "    \"beams\": ["
        "      {"
        "        \"name\": \"Laser\","
        "        \"cost\": 1,"
        "        \"tritanium\": 1,"
        "        \"duranium\": 0,"
        "        \"molybdenum\": 0,"
        "        \"mass\": 1,"
        "        \"techlevel\": 1,"
        "        \"crewkill\": 10,"
        "        \"damage\": 3,"
        "        \"id\": 1"
        "      },"
        "      {"
        "        \"name\": \"X-Ray Laser\","
        "        \"cost\": 2,"
        "        \"tritanium\": 1,"
        "        \"duranium\": 0,"
        "        \"molybdenum\": 0,"
        "        \"mass\": 1,"
        "        \"techlevel\": 1,"
        "        \"crewkill\": 15,"
        "        \"damage\": 1,"
        "        \"id\": 2"
        "      }"
        "    ],"
        "    \"engines\": ["
        "      {"
        "        \"name\": \"StarDrive 1\","
        "        \"cost\": 1,"
        "        \"tritanium\": 5,"
        "        \"duranium\": 1,"
        "        \"molybdenum\": 0,"
        "        \"techlevel\": 1,"
        "        \"warp1\": 100,"
        "        \"warp2\": 800,"
        "        \"warp3\": 2700,"
        "        \"warp4\": 6400,"
        "        \"warp5\": 12500,"
        "        \"warp6\": 21600,"
        "        \"warp7\": 34300,"
        "        \"warp8\": 51200,"
        "        \"warp9\": 72900,"
        "        \"id\": 1"
        "      },"
        "      {"
        "        \"name\": \"StarDrive 2\","
        "        \"cost\": 2,"
        "        \"tritanium\": 5,"
        "        \"duranium\": 2,"
        "        \"molybdenum\": 1,"
        "        \"techlevel\": 2,"
        "        \"warp1\": 100,"
        "        \"warp2\": 430,"
        "        \"warp3\": 2700,"
        "        \"warp4\": 6400,"
        "        \"warp5\": 12500,"
        "        \"warp6\": 21600,"
        "        \"warp7\": 34300,"
        "        \"warp8\": 51200,"
        "        \"warp9\": 72900,"
        "        \"id\": 2"
        "      }"
        "    ],"
        "    \"torpedos\": ["
        "      {"
        "        \"name\": \"Mark 1 Photon\","
        "        \"torpedocost\": 1,"
        "        \"launchercost\": 1,"
        "        \"tritanium\": 1,"
        "        \"duranium\": 1,"
        "        \"molybdenum\": 0,"
        "        \"mass\": 2,"
        "        \"techlevel\": 1,"
        "        \"crewkill\": 4,"
        "        \"damage\": 5,"
        "        \"id\": 1"
        "      },"
        "      {"
        "        \"name\": \"Proton Torp\","
        "        \"torpedocost\": 2,"
        "        \"launchercost\": 4,"
        "        \"tritanium\": 1,"
        "        \"duranium\": 0,"
        "        \"molybdenum\": 0,"
        "        \"mass\": 2,"
        "        \"techlevel\": 2,"
        "        \"crewkill\": 6,"
        "        \"damage\": 8,"
        "        \"combatrange\": 350,"      // Manually added for testing
        "        \"id\": 2"
        "      }"
        "    ],"
        "    \"advantages\": ["
        "      {"
        "        \"name\": \"Fed Crew Bonus\","
        "        \"description\": \"Fed text\","
        "        \"id\": 1"
        "      },"
        "      {"
        "        \"name\": \"Lizard Crew Bonus\","
        "        \"description\": \"Lizard text\","
        "        \"id\": 5"
        "      },"
        "      {"
        "        \"name\": \"Lizard Ground Bonus\","
        "        \"description\": \"Ground text\","
        "        \"id\": 6"
        "      },"
        "      {"
        "        \"name\": \"Dark Sense\","
        "        \"description\": \"Dark text\","
        "        \"id\": 22"
        "      },"
        "      {"
        "        \"name\": \"Quantum Torpedos\","
        "        \"description\": \"Quantum text\","
        "        \"id\": 79"
        "      }"
        "    ]"
        "  },"
        "  \"ispremium\": false"
        "}";

    // Environment
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    game::nu::Loader testee(tx, log);

    // Target objects
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    game::spec::ShipList shipList;
    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
        root->playerList().create(i);
    }

    // Test data
    std::auto_ptr<afl::data::Value> v(util::parseJSON(afl::string::toBytes(SPEC_FILE)));

    // Do it
    testee.loadShipList(shipList, *root, v);

    // Verify
    // -- advantages --
    const AdvantageList& advList = shipList.advantages();
    a.checkEqual("a01", advList.getNumAdvantages(), 5U);

    a.checkEqual("a11", advList.getName   (advList.getAdvantageByIndex(0)), "Fed Crew Bonus");
    a.checkEqual("a12", advList.getId     (advList.getAdvantageByIndex(0)), 1);
    a.checkEqual("a13", advList.getPlayers(advList.getAdvantageByIndex(0)), PlayerSet_t() + 1);

    a.checkEqual("a21", advList.getName   (advList.getAdvantageByIndex(1)), "Lizard Crew Bonus");
    a.checkEqual("a22", advList.getId     (advList.getAdvantageByIndex(1)), 5);
    a.checkEqual("a23", advList.getPlayers(advList.getAdvantageByIndex(1)), PlayerSet_t() + 2);

    a.checkEqual("a31", advList.getName   (advList.getAdvantageByIndex(2)), "Lizard Ground Bonus");
    a.checkEqual("a32", advList.getId     (advList.getAdvantageByIndex(2)), 6);
    a.checkEqual("a33", advList.getPlayers(advList.getAdvantageByIndex(2)), PlayerSet_t() + 2);

    a.checkEqual("a41", advList.getName   (advList.getAdvantageByIndex(3)), "Dark Sense");
    a.checkEqual("a42", advList.getId     (advList.getAdvantageByIndex(3)), 22);
    a.checkEqual("a43", advList.getPlayers(advList.getAdvantageByIndex(3)), PlayerSet_t() + 8);

    a.checkEqual("a51", advList.getName   (advList.getAdvantageByIndex(4)), "Quantum Torpedos");
    a.checkEqual("a52", advList.getId     (advList.getAdvantageByIndex(4)), 79);
    a.checkEqual("a53", advList.getPlayers(advList.getAdvantageByIndex(4)), PlayerSet_t() + 1 + 8);

    // -- config --
    a.checkEqual("c01", root->hostConfiguration()[HostConfiguration::GameName](), "Test Game");
    a.checkEqual("c02", root->hostConfiguration()[HostConfiguration::NumShips](), 400);
    a.checkEqual("c03", root->hostConfiguration()[HostConfiguration::ScanRange](1), 300);

    a.checkEqual("c11", root->hostConfiguration()[HostConfiguration::PlayerRace](1), 1);
    a.checkEqual("c11", root->hostConfiguration()[HostConfiguration::PlayerRace](2), 2);
    a.checkEqual("c11", root->hostConfiguration()[HostConfiguration::PlayerRace](8), 8);

    a.checkEqual("c21", root->hostConfiguration()[HostConfiguration::GroundKillFactor](1), 1);
    a.checkEqual("c22", root->hostConfiguration()[HostConfiguration::GroundKillFactor](2), 30);
    a.checkEqual("c23", root->hostConfiguration()[HostConfiguration::GroundKillFactor](8), 1);

    // -- race names --
    a.checkNonNull("p11", root->playerList().get(1));
    a.checkEqual("p12", root->playerList().get(1)->getName(Player::LongName, tx), "The Solar Federation");
    a.checkEqual("p13", root->playerList().get(1)->getName(Player::UserName, tx), "fed-player");

    a.checkNonNull("p21", root->playerList().get(2));
    a.checkEqual("p22", root->playerList().get(2)->getName(Player::LongName, tx), "The Lizard Alliance");
    a.checkEqual("p23", root->playerList().get(2)->getName(Player::UserName, tx), "lizard-player");

    a.checkNonNull("p81", root->playerList().get(8));
    a.checkEqual("p82", root->playerList().get(8)->getName(Player::LongName, tx), "The Evil Empire");
    a.checkEqual("p83", root->playerList().get(8)->getName(Player::UserName, tx), "ee-player");

    // -- hulls --
    a.checkGreaterThan("h01", shipList.hulls().size(), 70);

    a.checkNonNull("h11", shipList.hulls().get(1));
    a.checkEqual("h12", shipList.hulls().get(1)->getName(shipList.componentNamer()), "Outrider Class Scout");
    a.checkEqual("h13", shipList.hulls().get(1)->cost().get(Cost::Money), 50);
    a.checkEqual("h14", shipList.hulls().get(1)->cost().get(Cost::Tritanium), 40);
    a.checkEqual("h15", shipList.hulls().get(1)->getMaxCargo(), 40);
    a.checkEqual("h16", shipList.hulls().get(1)->getMaxFuel(), 260);
    a.checkEqual("h17", shipList.hulls().get(1)->getMaxBeams(), 1);
    a.checkEqual("h18", shipList.hulls().get(1)->getMaxLaunchers(), 0);
    a.checkEqual("h19", shipList.hulls().get(1)->getNumBays(), 0);
    a.checkEqual("h1a", shipList.hulls().get(1)->getNumEngines(), 1);

    a.checkNonNull("h21", shipList.hulls().get(15));
    a.checkEqual("h22", shipList.hulls().get(15)->getName(shipList.componentNamer()), "Small Deep Space Freighter");
    a.checkEqual("h23", shipList.hulls().get(15)->cost().get(Cost::Money), 10);
    a.checkEqual("h24", shipList.hulls().get(15)->cost().get(Cost::Tritanium), 2);
    a.checkEqual("h25", shipList.hulls().get(15)->getMaxCargo(), 70);
    a.checkEqual("h26", shipList.hulls().get(15)->getMaxFuel(), 200);
    a.checkEqual("h27", shipList.hulls().get(15)->getMaxBeams(), 0);
    a.checkEqual("h28", shipList.hulls().get(15)->getMaxLaunchers(), 0);
    a.checkEqual("h29", shipList.hulls().get(15)->getNumBays(), 0);
    a.checkEqual("h2a", shipList.hulls().get(15)->getNumEngines(), 1);

    a.checkNonNull("h31", shipList.hulls().get(71));
    a.checkEqual("h32", shipList.hulls().get(71)->getName(shipList.componentNamer()), "Ru25 Gunboat");
    a.checkEqual("h33", shipList.hulls().get(71)->cost().get(Cost::Money), 60);
    a.checkEqual("h34", shipList.hulls().get(71)->cost().get(Cost::Tritanium), 27);
    a.checkEqual("h35", shipList.hulls().get(71)->getMaxCargo(), 1);
    a.checkEqual("h36", shipList.hulls().get(71)->getMaxFuel(), 90);
    a.checkEqual("h37", shipList.hulls().get(71)->getMaxBeams(), 4);
    a.checkEqual("h38", shipList.hulls().get(71)->getMaxLaunchers(), 0);
    a.checkEqual("h39", shipList.hulls().get(71)->getNumBays(), 0);
    a.checkEqual("h3a", shipList.hulls().get(71)->getNumEngines(), 1);

    // -- beams --
    a.checkEqual("b01", shipList.beams().size(), 2);

    a.checkEqual("b11", shipList.beams().get(1)->getName(shipList.componentNamer()), "Laser");
    a.checkEqual("b12", shipList.beams().get(1)->cost().get(Cost::Money), 1);
    a.checkEqual("b13", shipList.beams().get(1)->cost().get(Cost::Tritanium), 1);
    a.checkEqual("b14", shipList.beams().get(1)->getKillPower(), 10);

    a.checkEqual("b21", shipList.beams().get(2)->getName(shipList.componentNamer()), "X-Ray Laser");
    a.checkEqual("b22", shipList.beams().get(2)->cost().get(Cost::Money), 2);
    a.checkEqual("b23", shipList.beams().get(2)->cost().get(Cost::Tritanium), 1);
    a.checkEqual("b24", shipList.beams().get(2)->getKillPower(), 15);

    // -- torpedoes --
    a.checkEqual("t01", shipList.launchers().size(), 2);

    a.checkEqual("t11", shipList.launchers().get(1)->getName(shipList.componentNamer()), "Mark 1 Photon");
    a.checkEqual("t12", shipList.launchers().get(1)->cost().get(Cost::Money), 1);
    a.checkEqual("t13", shipList.launchers().get(1)->cost().get(Cost::Tritanium), 1);
    a.checkEqual("t14", shipList.launchers().get(1)->torpedoCost().get(Cost::Tritanium), 1);
    a.checkEqual("t15", shipList.launchers().get(1)->torpedoCost().get(Cost::Money), 1);
    a.checkEqual("t16", shipList.launchers().get(1)->getKillPower(), 4);
    a.checkEqual("t17", shipList.launchers().get(1)->getFiringRangeBonus(), 0);

    a.checkEqual("t21", shipList.launchers().get(2)->getName(shipList.componentNamer()), "Proton Torp");
    a.checkEqual("t22", shipList.launchers().get(2)->cost().get(Cost::Money), 4);
    a.checkEqual("t23", shipList.launchers().get(2)->cost().get(Cost::Tritanium), 1);
    a.checkEqual("t24", shipList.launchers().get(2)->torpedoCost().get(Cost::Tritanium), 1);
    a.checkEqual("t25", shipList.launchers().get(2)->torpedoCost().get(Cost::Money), 2);
    a.checkEqual("t26", shipList.launchers().get(2)->getKillPower(), 6);
    a.checkEqual("t27", shipList.launchers().get(2)->getFiringRangeBonus(), 50);

    // -- engines --
    a.checkEqual("e01", shipList.engines().size(), 2);

    a.checkEqual("e11", shipList.engines().get(1)->getName(shipList.componentNamer()), "StarDrive 1");
    a.checkEqual("e12", shipList.engines().get(1)->cost().get(Cost::Tritanium), 5);
    a.checkEqual("e13", shipList.engines().get(1)->cost().get(Cost::Duranium), 1);
    a.checkEqual("e14", shipList.engines().get(1)->getFuelFactor(2).orElse(-1), 800);

    a.checkEqual("e21", shipList.engines().get(2)->getName(shipList.componentNamer()), "StarDrive 2");
    a.checkEqual("e22", shipList.engines().get(2)->cost().get(Cost::Tritanium), 5);
    a.checkEqual("e23", shipList.engines().get(2)->cost().get(Cost::Duranium), 2);
    a.checkEqual("e24", shipList.engines().get(2)->getFuelFactor(2).orElse(-1), 430);

    // -- hull assignments --
    a.checkEqual("x11", shipList.hullAssignments().getHullFromIndex(root->hostConfiguration(), 1, 1), 1);
    a.checkEqual("x12", shipList.hullAssignments().getHullFromIndex(root->hostConfiguration(), 1, 2), 0);

    a.checkEqual("x21", shipList.hullAssignments().getHullFromIndex(root->hostConfiguration(), 2, 1), 15);
    a.checkEqual("x22", shipList.hullAssignments().getHullFromIndex(root->hostConfiguration(), 2, 2), 0);

    a.checkEqual("x81", shipList.hullAssignments().getHullFromIndex(root->hostConfiguration(), 8, 1), 71);
    a.checkEqual("x82", shipList.hullAssignments().getHullFromIndex(root->hostConfiguration(), 8, 2), 15);
    a.checkEqual("x83", shipList.hullAssignments().getHullFromIndex(root->hostConfiguration(), 8, 3), 0);
}

/* Testing implicit advantages. Some are present even if not specified by RST. */
AFL_TEST("game.nu.Loader:loadShipList:implied", a)
{
    // Define all races, all players, all advantages, but no content.
    // Also test handling of a non 1:1 PlayerRace mapping.
    const char*const MIN_FILE =
        "{"
        "  \"success\": true,"
        "  \"rst\": {"
        "    \"settings\": {"
        "      \"quantumtorpedos\": true,"
        "      \"superspyadvanced\": true,"
        "      \"cloakandintercept\": true,"
        "      \"fascistdoublebeams\": true,"
        "      \"starbasefightertransfer\": true,"
        "      \"galacticpower\": true"
        "    },"
        "    \"races\": ["
        "      {\"id\":1},"
        "      {\"id\":2},"
        "      {\"id\":3},"
        "      {\"id\":4},"
        "      {\"id\":5},"
        "      {\"id\":6},"
        "      {\"id\":7},"
        "      {\"id\":8},"
        "      {\"id\":9},"
        "      {\"id\":10},"
        "      {\"id\":11}"
        "    ],"
        "    \"players\": ["
        "      {\"id\":1, \"raceid\":3},"
        "      {\"id\":2, \"raceid\":4},"
        "      {\"id\":3, \"raceid\":5},"
        "      {\"id\":4, \"raceid\":6},"
        "      {\"id\":5, \"raceid\":7},"
        "      {\"id\":6, \"raceid\":8},"
        "      {\"id\":7, \"raceid\":9},"
        "      {\"id\":8, \"raceid\":10},"
        "      {\"id\":9, \"raceid\":11},"
        "      {\"id\":10, \"raceid\":1},"
        "      {\"id\":11, \"raceid\":2}"
        "   ],"
        "   \"advantages\": ["
        "      {\"id\":36},"
        "      {\"id\":57},"
        "      {\"id\":62},"
        "      {\"id\":63},"
        "      {\"id\":77},"
        "      {\"id\":79}"
        "   ]"
        " }"
        "}";

    // Environment
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    game::nu::Loader testee(tx, log);

    // Target objects
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    game::spec::ShipList shipList;
    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
        root->playerList().create(i);
    }

    // Test data
    std::auto_ptr<afl::data::Value> v(util::parseJSON(afl::string::toBytes(MIN_FILE)));

    // Do it
    testee.loadShipList(shipList, *root, v);

    // Verify
    const AdvantageList& advList = shipList.advantages();
    a.checkEqual("adv 36", advList.getPlayers(advList.find(36)), PlayerSet_t() + 2);  // Klingons (player 2)
    a.checkEqual("adv 57", advList.getPlayers(advList.find(57)), PlayerSet_t() + 6);  // Empire (player 6)
    a.checkEqual("adv 62", advList.getPlayers(advList.find(62)), PlayerSet_t() + 1);  // Birds (player 1)
    a.checkEqual("adv 63", advList.getPlayers(advList.find(63)), PlayerSet_t() + 1);  // Birds (player 1)
    a.checkEqual("adv 77", advList.getPlayers(advList.find(77)), PlayerSet_t() + 6);  // Empire (player 6)
    a.checkEqual("adv 79", advList.getPlayers(advList.find(79)), PlayerSet_t() + 10); // Fed (player 10)

    a.checkEqual("pr 1", root->hostConfiguration()[HostConfiguration::PlayerRace](1), 3);
    a.checkEqual("pr 10", root->hostConfiguration()[HostConfiguration::PlayerRace](10), 1);
}

/* Testing advantages mapped to hullfuncs. */
AFL_TEST("game.nu.Loader:loadShipList:hullfunc", a)
{
    // Define all races, all players, all advantages, but no content.
    const char*const MIN_FILE =
        "{"
        "  \"success\": true,"
        "  \"rst\": {"
        "    \"races\": ["
        "      {\"id\":1, \"baseadvantages\":\"16\"},"
        "      {\"id\":2, \"baseadvantages\":\"16,28\"},"
        "      {\"id\":3, \"baseadvantages\":\"28\"}"
        "    ],"
        "    \"hulls\": ["
        "      {\"id\":1},"
        "      {\"id\":2, \"cancloak\":true},"
        "      {\"id\":3},"
        "      {\"id\":4}"
        "    ],"
        "    \"players\": ["
        "      {\"id\":1, \"raceid\":1},"
        "      {\"id\":2, \"raceid\":2},"
        "      {\"id\":3, \"raceid\":3}"
        "   ],"
        "   \"advantages\": ["
        "      {\"id\":16},"
        "      {\"id\":28}"
        "   ]"
        " }"
        "}";

    // Environment
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    game::nu::Loader testee(tx, log);

    // Target objects
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    game::spec::ShipList shipList;
    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
        root->playerList().create(i);
    }

    // Test data
    std::auto_ptr<afl::data::Value> v(util::parseJSON(afl::string::toBytes(MIN_FILE)));

    // Do it
    testee.loadShipList(shipList, *root, v);

    // Verify
    a.checkEqual("h1 board", getPlayersThatCan(*shipList.hulls().get(1), shipList, *root, BasicHullFunction::Boarding),       PlayerSet_t() + 1 + 2);
    a.checkEqual("h1 immu",  getPlayersThatCan(*shipList.hulls().get(1), shipList, *root, BasicHullFunction::PlanetImmunity), PlayerSet_t() + 2 + 3);
    a.checkEqual("h1 cloak", getPlayersThatCan(*shipList.hulls().get(1), shipList, *root, BasicHullFunction::Cloak),          PlayerSet_t());

    a.checkEqual("h2 board", getPlayersThatCan(*shipList.hulls().get(2), shipList, *root, BasicHullFunction::Boarding),       PlayerSet_t() + 1 + 2);
    a.checkEqual("h2 immu",  getPlayersThatCan(*shipList.hulls().get(2), shipList, *root, BasicHullFunction::PlanetImmunity), PlayerSet_t() + 2 + 3);
    a.checkEqual("h2 cloak", getPlayersThatCan(*shipList.hulls().get(2), shipList, *root, BasicHullFunction::Cloak),          PlayerSet_t::allUpTo(game::MAX_PLAYERS));
}

/* Test torps, combatrange field unset */
AFL_TEST("game.nu.Loader:loadShipList:torps:blank", a)
{
    // Define some torps with different specs
    const char*const MIN_FILE =
        "{"
        "  \"success\": true,"
        "  \"rst\": {"
        "    \"torpedos\": ["
        "      {\"id\":1},"
        "      {\"id\":10},"
        "      {\"id\":11}"
        "   ]"
        " }"
        "}";

    // Environment
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    game::nu::Loader testee(tx, log);

    // Target objects
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    game::spec::ShipList shipList;

    // Test data
    std::auto_ptr<afl::data::Value> v(util::parseJSON(afl::string::toBytes(MIN_FILE)));

    // Do it
    testee.loadShipList(shipList, *root, v);

    // Verify
    a.checkNonNull("t01", shipList.launchers().get(1));
    a.checkEqual("t02", shipList.launchers().get(1)->getFiringRangeBonus(), 0);

    a.checkNonNull("t11", shipList.launchers().get(10));
    a.checkEqual("t12", shipList.launchers().get(10)->getFiringRangeBonus(), 0);

    a.checkNonNull("t21", shipList.launchers().get(11));
    a.checkEqual("t22", shipList.launchers().get(11)->getFiringRangeBonus(), 40);
}

/* Test torps, combatrange field set */
AFL_TEST("game.nu.Loader:loadShipList:torps:set", a)
{
    // Define some torps with different specs
    const char*const MIN_FILE =
        "{"
        "  \"success\": true,"
        "  \"rst\": {"
        "    \"torpedos\": ["
        "      {\"id\":1,\"combatrange\":100},"
        "      {\"id\":10,\"combatrange\":300},"
        "      {\"id\":11,\"combatrange\":400}"
        "   ]"
        " }"
        "}";

    // Environment
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    game::nu::Loader testee(tx, log);

    // Target objects
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    game::spec::ShipList shipList;

    // Test data
    std::auto_ptr<afl::data::Value> v(util::parseJSON(afl::string::toBytes(MIN_FILE)));

    // Do it
    testee.loadShipList(shipList, *root, v);

    // Verify
    a.checkNonNull("t01", shipList.launchers().get(1));
    a.checkEqual("t02", shipList.launchers().get(1)->getFiringRangeBonus(), -200);

    a.checkNonNull("t11", shipList.launchers().get(10));
    a.checkEqual("t12", shipList.launchers().get(10)->getFiringRangeBonus(), 0);

    a.checkNonNull("t21", shipList.launchers().get(11));
    a.checkEqual("t22", shipList.launchers().get(11)->getFiringRangeBonus(), 100);
}

/* Coarse general test for loadTurn */
AFL_TEST("game.nu.Loader:loadTurn", a)
{
    const char* const FILE =
        "{"
        "  \"success\": true,"
        "  \"rst\": {"
        "    \"settings\": {"
        "      \"hostcompleted\": \"4/12/2012 9:04:45 PM\""
        "    },"
        "    \"game\": {"
        "      \"turn\": 90"
        "    },"
        "    \"planets\": ["
        "      {"
        "        \"name\": \"Ceti Alpha one\","
        "        \"x\": 1766,"
        "        \"y\": 2091,"
        "        \"friendlycode\": \"???\","
        "        \"mines\": -1,"
        "        \"factories\": -1,"
        "        \"defense\": -1,"
        "        \"targetmines\": 0,"
        "        \"targetfactories\": 0,"
        "        \"targetdefense\": 0,"
        "        \"builtmines\": 0,"
        "        \"builtfactories\": 0,"
        "        \"builtdefense\": 0,"
        "        \"buildingstarbase\": false,"
        "        \"megacredits\": -1,"
        "        \"supplies\": -1,"
        "        \"suppliessold\": 0,"
        "        \"neutronium\": -1,"
        "        \"molybdenum\": -1,"
        "        \"duranium\": -1,"
        "        \"tritanium\": -1,"
        "        \"groundneutronium\": -1,"
        "        \"groundmolybdenum\": -1,"
        "        \"groundduranium\": -1,"
        "        \"groundtritanium\": -1,"
        "        \"densityneutronium\": -1,"
        "        \"densitymolybdenum\": -1,"
        "        \"densityduranium\": -1,"
        "        \"densitytritanium\": -1,"
        "        \"totalneutronium\": 0,"
        "        \"totalmolybdenum\": 0,"
        "        \"totalduranium\": 0,"
        "        \"totaltritanium\": 0,"
        "        \"checkneutronium\": -1,"
        "        \"checkmolybdenum\": -1,"
        "        \"checkduranium\": -1,"
        "        \"checktritanium\": -1,"
        "        \"checkmegacredits\": -1,"
        "        \"checksupplies\": -1,"
        "        \"temp\": 37,"
        "        \"ownerid\": 8,"
        "        \"clans\": -1,"
        "        \"colchange\": 0,"
        "        \"colonisttaxrate\": 0,"
        "        \"colonisthappypoints\": 0,"
        "        \"colhappychange\": 0,"
        "        \"nativeclans\": -1,"
        "        \"nativechange\": 0,"
        "        \"nativegovernment\": 0,"
        "        \"nativetaxvalue\": 0,"
        "        \"nativetype\": 0,"
        "        \"nativetaxrate\": 0,"
        "        \"nativehappypoints\": 0,"
        "        \"nativehappychange\": 0,"
        "        \"infoturn\": 89,"
        "        \"debrisdisk\": 0,"
        "        \"flag\": 0,"
        "        \"readystatus\": 0,"
        "        \"targetx\": 0,"
        "        \"targety\": 0,"
        "        \"podhullid\": 0,"
        "        \"podspeed\": 0,"
        "        \"podcargo\": 0,"
        "        \"larva\": 0,"
        "        \"larvaturns\": 0,"
        "        \"img\": \"http://library.vgaplanets.nu/planets/37.png\","
        "        \"nativeracename\": \"none\","
        "        \"nativegovernmentname\": \"?\","
        "        \"id\": 1"
        "      },"
        "      {"
        "        \"name\": \"Orionis I\","
        "        \"x\": 2550,"
        "        \"y\": 1703,"
        "        \"friendlycode\": \"qwe\","
        "        \"mines\": 75,"
        "        \"factories\": 130,"
        "        \"defense\": 80,"
        "        \"targetmines\": 0,"
        "        \"targetfactories\": 0,"
        "        \"targetdefense\": 0,"
        "        \"builtmines\": 0,"
        "        \"builtfactories\": 0,"
        "        \"builtdefense\": 0,"
        "        \"buildingstarbase\": false,"
        "        \"megacredits\": 0,"
        "        \"supplies\": 130,"
        "        \"suppliessold\": 0,"
        "        \"neutronium\": 5,"
        "        \"molybdenum\": 3,"
        "        \"duranium\": 4,"
        "        \"tritanium\": 3,"
        "        \"groundneutronium\": 5,"
        "        \"groundmolybdenum\": 3,"
        "        \"groundduranium\": 4,"
        "        \"groundtritanium\": 3,"
        "        \"densityneutronium\": 94,"
        "        \"densitymolybdenum\": 58,"
        "        \"densityduranium\": 71,"
        "        \"densitytritanium\": 43,"
        "        \"totalneutronium\": 0,"
        "        \"totalmolybdenum\": 0,"
        "        \"totalduranium\": 0,"
        "        \"totaltritanium\": 0,"
        "        \"checkneutronium\": 5,"
        "        \"checkmolybdenum\": 3,"
        "        \"checkduranium\": 4,"
        "        \"checktritanium\": 3,"
        "        \"checkmegacredits\": 1490,"
        "        \"checksupplies\": 415,"
        "        \"temp\": 74,"
        "        \"ownerid\": 7,"
        "        \"clans\": 1037,"
        "        \"colchange\": 0,"
        "        \"colonisttaxrate\": 0,"
        "        \"colonisthappypoints\": 100,"
        "        \"colhappychange\": 8,"
        "        \"nativeclans\": 0,"
        "        \"nativechange\": 0,"
        "        \"nativegovernment\": 0,"
        "        \"nativetaxvalue\": 0,"
        "        \"nativetype\": 0,"
        "        \"nativetaxrate\": 0,"
        "        \"nativehappypoints\": 80,"
        "        \"nativehappychange\": 3,"
        "        \"infoturn\": 90,"
        "        \"debrisdisk\": 0,"
        "        \"flag\": 0,"
        "        \"readystatus\": 0,"
        "        \"targetx\": 0,"
        "        \"targety\": 0,"
        "        \"podhullid\": 0,"
        "        \"podspeed\": 0,"
        "        \"podcargo\": 0,"
        "        \"larva\": 0,"
        "        \"larvaturns\": 0,"
        "        \"img\": \"http://library.vgaplanets.nu/planets/174.png\","
        "        \"nativeracename\": \"none\","
        "        \"nativegovernmentname\": \"?\","
        "        \"id\": 2"
        "      },"
        "      {"
        "        \"name\": \"Fussbar\","
        "        \"x\": 2621,"
        "        \"y\": 2041,"
        "        \"friendlycode\": \"sdu\","
        "        \"mines\": 75,"
        "        \"factories\": 227,"
        "        \"defense\": 177,"
        "        \"targetmines\": 0,"
        "        \"targetfactories\": 0,"
        "        \"targetdefense\": 0,"
        "        \"builtmines\": 0,"
        "        \"builtfactories\": 0,"
        "        \"builtdefense\": 0,"
        "        \"buildingstarbase\": false,"
        "        \"megacredits\": 1519,"
        "        \"supplies\": 1517,"
        "        \"suppliessold\": 0,"
        "        \"neutronium\": 280,"
        "        \"molybdenum\": 42,"
        "        \"duranium\": 141,"
        "        \"tritanium\": 87,"
        "        \"groundneutronium\": 652,"
        "        \"groundmolybdenum\": 3,"
        "        \"groundduranium\": 5,"
        "        \"groundtritanium\": 4,"
        "        \"densityneutronium\": 74,"
        "        \"densitymolybdenum\": 50,"
        "        \"densityduranium\": 91,"
        "        \"densitytritanium\": 76,"
        "        \"totalneutronium\": 0,"
        "        \"totalmolybdenum\": 0,"
        "        \"totalduranium\": 0,"
        "        \"totaltritanium\": 0,"
        "        \"checkneutronium\": 280,"
        "        \"checkmolybdenum\": 381,"
        "        \"checkduranium\": 317,"
        "        \"checktritanium\": 226,"
        "        \"checkmegacredits\": 15608,"
        "        \"checksupplies\": 1996,"
        "        \"temp\": 79,"
        "        \"ownerid\": 7,"
        "        \"clans\": 16925,"
        "        \"colchange\": 0,"
        "        \"colonisttaxrate\": 0,"
        "        \"colonisthappypoints\": 100,"
        "        \"colhappychange\": 7,"
        "        \"nativeclans\": 91914,"
        "        \"nativechange\": 0,"
        "        \"nativegovernment\": 7,"
        "        \"nativetaxvalue\": 0,"
        "        \"nativetype\": 1,"
        "        \"nativetaxrate\": 5,"
        "        \"nativehappypoints\": 70,"
        "        \"nativehappychange\": 0,"
        "        \"infoturn\": 90,"
        "        \"debrisdisk\": 0,"
        "        \"flag\": 0,"
        "        \"readystatus\": 0,"
        "        \"targetx\": 0,"
        "        \"targety\": 0,"
        "        \"podhullid\": 0,"
        "        \"podspeed\": 0,"
        "        \"podcargo\": 0,"
        "        \"larva\": 0,"
        "        \"larvaturns\": 0,"
        "        \"img\": \"http://library.vgaplanets.nu/planets/79.png\","
        "        \"nativeracename\": \"Humanoid\","
        "        \"nativegovernmentname\": \"Representative\","
        "        \"id\": 5"
        "      },"
        "      {"
        "        \"name\": \"Wayne's World\","
        "        \"x\": 2282,"
        "        \"y\": 1255,"
        "        \"friendlycode\": \"126\","
        "        \"mines\": 30,"
        "        \"factories\": 110,"
        "        \"defense\": 66,"
        "        \"targetmines\": 0,"
        "        \"targetfactories\": 0,"
        "        \"targetdefense\": 0,"
        "        \"builtmines\": 0,"
        "        \"builtfactories\": 0,"
        "        \"builtdefense\": 0,"
        "        \"buildingstarbase\": false,"
        "        \"megacredits\": 307,"
        "        \"supplies\": 510,"
        "        \"suppliessold\": 0,"
        "        \"neutronium\": 382,"
        "        \"molybdenum\": 218,"
        "        \"duranium\": 2125,"
        "        \"tritanium\": 313,"
        "        \"groundneutronium\": 2,"
        "        \"groundmolybdenum\": 2,"
        "        \"groundduranium\": 4,"
        "        \"groundtritanium\": 901,"
        "        \"densityneutronium\": 22,"
        "        \"densitymolybdenum\": 34,"
        "        \"densityduranium\": 65,"
        "        \"densitytritanium\": 43,"
        "        \"totalneutronium\": 0,"
        "        \"totalmolybdenum\": 0,"
        "        \"totalduranium\": 0,"
        "        \"totaltritanium\": 0,"
        "        \"checkneutronium\": 382,"
        "        \"checkmolybdenum\": 377,"
        "        \"checkduranium\": 2174,"
        "        \"checktritanium\": 561,"
        "        \"checkmegacredits\": 13577,"
        "        \"checksupplies\": 716,"
        "        \"temp\": 59,"
        "        \"ownerid\": 11,"
        "        \"clans\": 355,"
        "        \"colchange\": 0,"
        "        \"colonisttaxrate\": 0,"
        "        \"colonisthappypoints\": 100,"
        "        \"colhappychange\": 9,"
        "        \"nativeclans\": 59027,"
        "        \"nativechange\": 0,"
        "        \"nativegovernment\": 5,"
        "        \"nativetaxvalue\": 0,"
        "        \"nativetype\": 9,"
        "        \"nativetaxrate\": 4,"
        "        \"nativehappypoints\": 100,"
        "        \"nativehappychange\": 0,"
        "        \"infoturn\": 90,"
        "        \"debrisdisk\": 0,"
        "        \"flag\": 0,"
        "        \"readystatus\": 2,"
        "        \"targetx\": 0,"
        "        \"targety\": 0,"
        "        \"podhullid\": 0,"
        "        \"podspeed\": 0,"
        "        \"podcargo\": 0,"
        "        \"larva\": 0,"
        "        \"larvaturns\": 0,"
        "        \"img\": \"http://library.vgaplanets.nu/planets/159.png\","
        "        \"nativeracename\": \"Siliconoid\","
        "        \"nativegovernmentname\": \"Feudal\","
        "        \"id\": 12"
        "      }],"
        "    \"ships\": ["
        "      {"
        "        \"friendlycode\": \"\","
        "        \"name\": \"Queen\","
        "        \"warp\": 9,"
        "        \"x\": 2503,"
        "        \"y\": 1193,"
        "        \"beams\": 0,"
        "        \"bays\": 0,"
        "        \"torps\": 0,"
        "        \"mission\": 0,"
        "        \"mission1target\": 0,"
        "        \"mission2target\": 0,"
        "        \"enemy\": 0,"
        "        \"damage\": 0,"
        "        \"crew\": -1,"
        "        \"clans\": 0,"
        "        \"neutronium\": 250,"
        "        \"tritanium\": 0,"
        "        \"duranium\": 0,"
        "        \"molybdenum\": 0,"
        "        \"supplies\": 0,"
        "        \"ammo\": 0,"
        "        \"megacredits\": 0,"
        "        \"transferclans\": 0,"
        "        \"transferneutronium\": 0,"
        "        \"transferduranium\": 0,"
        "        \"transfertritanium\": 0,"
        "        \"transfermolybdenum\": 0,"
        "        \"transfersupplies\": 0,"
        "        \"transferammo\": 0,"
        "        \"transfermegacredits\": 0,"
        "        \"transfertargetid\": 0,"
        "        \"transfertargettype\": 0,"
        "        \"targetx\": 2503,"
        "        \"targety\": 1193,"
        "        \"mass\": 310,"
        "        \"heading\": -1,"
        "        \"turn\": 0,"
        "        \"turnkilled\": 0,"
        "        \"beamid\": 0,"
        "        \"engineid\": 9,"
        "        \"hullid\": 16,"
        "        \"ownerid\": 11,"
        "        \"torpedoid\": 0,"
        "        \"experience\": 0,"
        "        \"infoturn\": 90,"
        "        \"podhullid\": 0,"
        "        \"podcargo\": 0,"
        "        \"goal\": 0,"
        "        \"goaltarget\": 0,"
        "        \"goaltarget2\": 0,"
        "        \"waypoints\": [],"
        "        \"history\": [],"
        "        \"iscloaked\": false,"
        "        \"readystatus\": 0,"
        "        \"id\": 6"
        "      },"
        "      {"
        "        \"friendlycode\": \"ehm\","
        "        \"name\": \"Augsburg\","
        "        \"warp\": 9,"
        "        \"x\": 2609,"
        "        \"y\": 1745,"
        "        \"beams\": 0,"
        "        \"bays\": 0,"
        "        \"torps\": 0,"
        "        \"mission\": 4,"
        "        \"mission1target\": 0,"
        "        \"mission2target\": 0,"
        "        \"enemy\": 0,"
        "        \"damage\": 0,"
        "        \"crew\": 6,"
        "        \"clans\": 0,"
        "        \"neutronium\": 60,"
        "        \"tritanium\": 0,"
        "        \"duranium\": 0,"
        "        \"molybdenum\": 200,"
        "        \"supplies\": 0,"
        "        \"ammo\": 0,"
        "        \"megacredits\": 0,"
        "        \"transferclans\": 0,"
        "        \"transferneutronium\": 0,"
        "        \"transferduranium\": 0,"
        "        \"transfertritanium\": 0,"
        "        \"transfermolybdenum\": 0,"
        "        \"transfersupplies\": 0,"
        "        \"transferammo\": 0,"
        "        \"transfermegacredits\": 0,"
        "        \"transfertargetid\": 0,"
        "        \"transfertargettype\": 0,"
        "        \"targetx\": 2607,"
        "        \"targety\": 1747,"
        "        \"mass\": 320,"
        "        \"heading\": 315,"
        "        \"turn\": 2,"
        "        \"turnkilled\": 0,"
        "        \"beamid\": 0,"
        "        \"engineid\": 9,"
        "        \"hullid\": 16,"
        "        \"ownerid\": 7,"
        "        \"torpedoid\": 0,"
        "        \"experience\": 53,"
        "        \"infoturn\": 90,"
        "        \"podhullid\": 0,"
        "        \"podcargo\": 0,"
        "        \"goal\": 0,"
        "        \"goaltarget\": 0,"
        "        \"goaltarget2\": 0,"
        "        \"waypoints\": [],"
        "        \"history\": [],"
        "        \"iscloaked\": false,"
        "        \"readystatus\": 0,"
        "        \"id\": 12"
        "      }"
        "    ],"
        "    \"ionstorms\": ["
        "      {"
        "        \"x\": 1414,"
        "        \"y\": 1438,"
        "        \"radius\": 166,"
        "        \"voltage\": 104,"
        "        \"warp\": 6,"
        "        \"heading\": 234,"
        "        \"isgrowing\": false,"
        "        \"parentid\": 0,"
        "        \"id\": 56"
        "      }"
        "    ],"
        "    \"starbases\": ["
        "      {"
        "        \"defense\": 50,"
        "        \"builtdefense\": 0,"
        "        \"damage\": 0,"
        "        \"enginetechlevel\": 7,"
        "        \"hulltechlevel\": 10,"
        "        \"beamtechlevel\": 5,"
        "        \"torptechlevel\": 1,"
        "        \"hulltechup\": 0,"
        "        \"enginetechup\": 0,"
        "        \"beamtechup\": 0,"
        "        \"torptechup\": 0,"
        "        \"fighters\": 20,"
        "        \"builtfighters\": 0,"
        "        \"shipmission\": 0,"
        "        \"mission\": 6,"
        "        \"mission1target\": 0,"
        "        \"planetid\": 5,"
        "        \"raceid\": 0,"
        "        \"targetshipid\": 0,"
        "        \"buildbeamid\": 6,"
        "        \"buildengineid\": 7,"
        "        \"buildtorpedoid\": 0,"
        "        \"buildhullid\": 67,"
        "        \"buildbeamcount\": 6,"
        "        \"buildtorpcount\": 0,"
        "        \"isbuilding\": true,"
        "        \"starbasetype\": 0,"
        "        \"infoturn\": 90,"
        "        \"readystatus\": 0,"
        "        \"id\": 169"
        "      }"
        "    ],"
        "    \"stock\": ["
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 14,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12805"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 15,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12803"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 16,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12806"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 17,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12811"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 18,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12816"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 59,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12809"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 60,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12807"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 61,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12812"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 62,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12810"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 63,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12814"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 64,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12813"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 65,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12808"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 66,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12804"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 67,"
        "        \"amount\": 1,"
        "        \"builtamount\": 0,"
        "        \"id\": 12817"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 104,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12815"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 1,"
        "        \"stockid\": 105,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12818"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 2,"
        "        \"stockid\": 1,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12819"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 2,"
        "        \"stockid\": 2,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12820"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 2,"
        "        \"stockid\": 3,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12821"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 2,"
        "        \"stockid\": 4,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12822"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 2,"
        "        \"stockid\": 5,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12823"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 2,"
        "        \"stockid\": 6,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12824"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 2,"
        "        \"stockid\": 7,"
        "        \"amount\": 4,"
        "        \"builtamount\": 0,"
        "        \"id\": 12825"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 2,"
        "        \"stockid\": 8,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12826"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 2,"
        "        \"stockid\": 9,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12827"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 3,"
        "        \"stockid\": 1,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12828"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 3,"
        "        \"stockid\": 2,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12829"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 3,"
        "        \"stockid\": 3,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12830"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 3,"
        "        \"stockid\": 4,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12831"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 3,"
        "        \"stockid\": 5,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12832"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 3,"
        "        \"stockid\": 6,"
        "        \"amount\": 6,"
        "        \"builtamount\": 0,"
        "        \"id\": 12833"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 3,"
        "        \"stockid\": 7,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12834"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 3,"
        "        \"stockid\": 8,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12835"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 3,"
        "        \"stockid\": 9,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12836"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 3,"
        "        \"stockid\": 10,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12837"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 4,"
        "        \"stockid\": 1,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12838"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 4,"
        "        \"stockid\": 2,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12840"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 4,"
        "        \"stockid\": 3,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12842"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 4,"
        "        \"stockid\": 4,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12844"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 4,"
        "        \"stockid\": 5,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12846"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 4,"
        "        \"stockid\": 6,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12848"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 4,"
        "        \"stockid\": 7,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12850"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 4,"
        "        \"stockid\": 8,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12852"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 4,"
        "        \"stockid\": 9,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12854"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 4,"
        "        \"stockid\": 10,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12856"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 5,"
        "        \"stockid\": 1,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12839"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 5,"
        "        \"stockid\": 2,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12841"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 5,"
        "        \"stockid\": 3,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12843"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 5,"
        "        \"stockid\": 4,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12845"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 5,"
        "        \"stockid\": 5,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12847"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 5,"
        "        \"stockid\": 6,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12849"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 5,"
        "        \"stockid\": 7,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12851"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 5,"
        "        \"stockid\": 8,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12853"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 5,"
        "        \"stockid\": 9,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12855"
        "      },"
        "      {"
        "        \"starbaseid\": 169,"
        "        \"stocktype\": 5,"
        "        \"stockid\": 10,"
        "        \"amount\": 0,"
        "        \"builtamount\": 0,"
        "        \"id\": 12857"
        "      }"
        "    ],"
        "    \"minefields\": ["
        "      {"
        "        \"ownerid\": 9,"
        "        \"isweb\": false,"
        "        \"units\": 1566,"
        "        \"infoturn\": 89,"
        "        \"friendlycode\": \"???\","
        "        \"x\": 2040,"
        "        \"y\": 2711,"
        "        \"radius\": 39,"
        "        \"id\": 1"
        "      },"
        "      {"
        "        \"ownerid\": 7,"
        "        \"isweb\": true,"
        "        \"units\": 2399,"
        "        \"infoturn\": 90,"
        "        \"friendlycode\": \"ftg\","
        "        \"x\": 2683,"
        "        \"y\": 1732,"
        "        \"radius\": 48,"
        "        \"id\": 5"
        "      }"
        "    ],"
        "    \"vcrs\": ["
        "      {"
        "        \"seed\": 111,"
        "        \"x\": 2128,"
        "        \"y\": 1603,"
        "        \"battletype\": 1,"
        "        \"leftownerid\": 7,"
        "        \"rightownerid\": 1,"
        "        \"turn\": 90,"
        "        \"id\": 371,"
        "        \"left\": {"
        "          \"vcrid\": 371,"
        "          \"objectid\": 328,"
        "          \"name\": \"CCSS KILA\","
        "          \"side\": 0,"
        "          \"beamcount\": 4,"
        "          \"launchercount\": 0,"
        "          \"baycount\": 6,"
        "          \"hullid\": 19,"
        "          \"beamid\": 9,"
        "          \"torpedoid\": 10,"
        "          \"shield\": 100,"
        "          \"damage\": 0,"
        "          \"crew\": 370,"
        "          \"mass\": 173,"
        "          \"raceid\": 7,"
        "          \"beamkillbonus\": 1,"
        "          \"beamchargerate\": 1,"
        "          \"torpchargerate\": 1,"
        "          \"torpmisspercent\": 1,"
        "          \"crewdefensepercent\": 0,"
        "          \"torpedos\": 0,"
        "          \"fighters\": 54,"
        "          \"temperature\": 0,"
        "          \"hasstarbase\": false,"
        "          \"id\": 741"
        "        },"
        "        \"right\": {"
        "          \"vcrid\": 371,"
        "          \"objectid\": 235,"
        "          \"name\": \"Challenger 5\","
        "          \"side\": 1,"
        "          \"beamcount\": 1,"
        "          \"launchercount\": 0,"
        "          \"baycount\": 1,"
        "          \"hullid\": 0,"
        "          \"beamid\": 1,"
        "          \"torpedoid\": 0,"
        "          \"shield\": 100,"
        "          \"damage\": 0,"
        "          \"crew\": 10000,"
        "          \"mass\": 101,"
        "          \"raceid\": 1,"
        "          \"beamkillbonus\": 1,"
        "          \"beamchargerate\": 1,"
        "          \"torpchargerate\": 1,"
        "          \"torpmisspercent\": 1,"
        "          \"crewdefensepercent\": 0,"
        "          \"torpedos\": 0,"
        "          \"fighters\": 1,"
        "          \"temperature\": 21,"
        "          \"hasstarbase\": false,"
        "          \"id\": 742"
        "        }"
        "      }"
        "    ]"
        "  }"
        "}";

    // Environment
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    game::nu::Loader testee(tx, log);

    // Target object
    game::Turn turn;

    // Test data
    std::auto_ptr<afl::data::Value> v(util::parseJSON(afl::string::toBytes(FILE)));
    testee.loadTurn(turn, PlayerSet_t() + 7, v);

    // Postprocess. Required for starbase status.
    game::map::Configuration mapConfig;
    game::HostVersion hostVersion;
    game::config::HostConfiguration hostConfig;
    game::spec::ShipList shipList;
    turn.universe().postprocess(PlayerSet_t() + 7, PlayerSet_t() + 7, game::map::Object::Playable, mapConfig, hostVersion, hostConfig, turn.getTurnNumber(), shipList, tx, log);

    // Verify
    // - turn metadata -
    a.checkEqual("t01", turn.getTurnNumber(), 90);
    a.checkEqual("t02", turn.getTimestamp().getTimestampAsString(), "04-12-201221:04:45");

    // - planets/bases -
    a.checkNonNull("p01", turn.universe().planets().get(1));
    a.checkEqual("p02", turn.universe().planets().get(1)->getName(tx), "Ceti Alpha one");
    a.checkEqual("p03", turn.universe().planets().get(1)->getCargo(Element::Neutronium).isValid(), false);
    // FIXME: not yet true: a.checkEqual("p04", turn.universe().planets().get(1)->getOwner().orElse(99), 8);
    a.checkEqual("p04", turn.universe().planets().get(1)->hasBase(), false);

    a.checkNonNull("p11", turn.universe().planets().get(2));
    a.checkEqual("p12", turn.universe().planets().get(2)->getName(tx), "Orionis I");
    a.checkEqual("p13", turn.universe().planets().get(2)->getCargo(Element::Neutronium).orElse(99), 5);
    a.checkEqual("p14", turn.universe().planets().get(2)->getOwner().orElse(99), 7);
    a.checkEqual("p14", turn.universe().planets().get(2)->hasBase(), false);

    a.checkNonNull("p21", turn.universe().planets().get(5));
    a.checkEqual("p22", turn.universe().planets().get(5)->getName(tx), "Fussbar");
    a.checkEqual("p23", turn.universe().planets().get(5)->getCargo(Element::Neutronium).orElse(99), 280);
    a.checkEqual("p24", turn.universe().planets().get(5)->getOwner().orElse(99), 7);
    a.checkEqual("p24", turn.universe().planets().get(5)->hasBase(), true);
    a.checkEqual("p25", turn.universe().planets().get(5)->getNumBuildings(game::BaseDefenseBuilding).orElse(99), 50);

    a.checkNonNull("p31", turn.universe().planets().get(12));
    a.checkEqual("p32", turn.universe().planets().get(12)->getName(tx), "Wayne's World");
    a.checkEqual("p33", turn.universe().planets().get(12)->getCargo(Element::Neutronium).orElse(99), 382);
    a.checkEqual("p34", turn.universe().planets().get(12)->getOwner().orElse(99), 11);
    a.checkEqual("p34", turn.universe().planets().get(12)->hasBase(), false);

    // - ships -
    a.checkNonNull("s01", turn.universe().ships().get(6));
    a.checkEqual("s02", turn.universe().ships().get(6)->getName(), "Queen");
    a.checkEqual("s03", turn.universe().ships().get(6)->getOwner().orElse(99), 11);

    a.checkNonNull("s11", turn.universe().ships().get(12));
    a.checkEqual("s12", turn.universe().ships().get(12)->getName(), "Augsburg");
    a.checkEqual("s13", turn.universe().ships().get(12)->getOwner().orElse(99), 7);
    a.checkEqual("s14", turn.universe().ships().get(12)->getCrew().orElse(99), 6);
    a.checkEqual("s15", turn.universe().ships().get(12)->getCargo(Element::Neutronium).orElse(99), 60);
    a.checkEqual("s16", turn.universe().ships().get(12)->getFriendlyCode().orElse(""), "ehm");

    // - minefields -
    a.checkNonNull("m01", turn.universe().minefields().get(1));
    a.checkEqual("m02", turn.universe().minefields().get(1)->getUnits(), 1487);      // After decay, minefield scan is one turn old.
    a.checkEqual("m03", turn.universe().minefields().get(1)->getRadius().orElse(-1), 38);
    a.checkEqual("m04", turn.universe().minefields().get(1)->getOwner().orElse(-1), 9);
    a.checkEqual("m05", turn.universe().minefields().get(1)->isWeb(), false);

    a.checkNonNull("m11", turn.universe().minefields().get(5));
    a.checkEqual("m12", turn.universe().minefields().get(5)->getUnits(), 2399);
    a.checkEqual("m13", turn.universe().minefields().get(5)->getRadius().orElse(-1), 48);
    a.checkEqual("m14", turn.universe().minefields().get(5)->getOwner().orElse(-1), 7);
    a.checkEqual("m15", turn.universe().minefields().get(5)->isWeb(), true);

    // - storms -
    a.checkNonNull("i01", turn.universe().ionStorms().get(56));
    a.checkEqual("i02", turn.universe().ionStorms().get(56)->getRadius().orElse(-1), 166);
    a.checkEqual("i02", turn.universe().ionStorms().get(56)->getVoltage().orElse(-1), 104);

    // - vcrs -
    a.checkNonNull("v01", turn.getBattles().get());
    a.checkEqual("v02", turn.getBattles()->getNumBattles(), 1U);

    a.checkNonNull("v11", turn.getBattles()->getBattle(0));
    a.checkEqual("v12", turn.getBattles()->getBattle(0)->getNumObjects(), 2U);

    a.checkNonNull("v21", turn.getBattles()->getBattle(0)->getObject(0, false));
    a.checkEqual("v22", turn.getBattles()->getBattle(0)->getObject(0, false)->getName(), "CCSS KILA");
    a.checkEqual("v23", turn.getBattles()->getBattle(0)->getObject(0, false)->getNumBeams(), 4);
    a.checkEqual("v24", turn.getBattles()->getBattle(0)->getObject(0, false)->isPlanet(), false);
    a.checkEqual("v25", turn.getBattles()->getBattle(0)->getObject(0, false)->getOwner(), 7);

    a.checkNonNull("v31", turn.getBattles()->getBattle(0)->getObject(1, false));
    a.checkEqual("v32", turn.getBattles()->getBattle(0)->getObject(1, false)->getName(), "Challenger 5");
    a.checkEqual("v33", turn.getBattles()->getBattle(0)->getObject(1, false)->getNumBeams(), 1);
    a.checkEqual("v34", turn.getBattles()->getBattle(0)->getObject(1, false)->isPlanet(), true);
    a.checkEqual("v35", turn.getBattles()->getBattle(0)->getObject(1, false)->getOwner(), 1);
}

/* Test loadTime */
AFL_TEST("game.nu.Loader:loadTime", a)
{
    std::auto_ptr<afl::data::Value> p;

    // Empty
    a.checkEqual("01", game::nu::Loader::loadTime(p).getTimestampAsString(), "00-00-000000:00:00");

    // Bad types
    p.reset(new afl::data::IntegerValue(42));
    a.checkEqual("02", game::nu::Loader::loadTime(p).getTimestampAsString(), "00-00-000000:00:00");

    p.reset(new afl::data::StringValue("xyz"));
    a.checkEqual("03", game::nu::Loader::loadTime(p).getTimestampAsString(), "00-00-000000:00:00");
    p.reset(new afl::data::StringValue("4/12/2012 12:04:45"));  // incomplete
    a.checkEqual("04", game::nu::Loader::loadTime(p).getTimestampAsString(), "00-00-000000:00:00");
    p.reset(new afl::data::StringValue("4.12.2012 12:04:45 AM"));  // bad separator
    a.checkEqual("05", game::nu::Loader::loadTime(p).getTimestampAsString(), "00-00-000000:00:00");

    // Normal
    p.reset(new afl::data::StringValue("4/12/2012 12:04:45 AM"));
    a.checkEqual("11", game::nu::Loader::loadTime(p).getTimestampAsString(), "04-12-201200:04:45");
    p.reset(new afl::data::StringValue("4/12/2012 9:04:45 AM"));
    a.checkEqual("12", game::nu::Loader::loadTime(p).getTimestampAsString(), "04-12-201209:04:45");
    p.reset(new afl::data::StringValue("4/12/2012 12:04:45 PM"));
    a.checkEqual("13", game::nu::Loader::loadTime(p).getTimestampAsString(), "04-12-201212:04:45");
    p.reset(new afl::data::StringValue("4/12/2012 9:04:45 PM"));
    a.checkEqual("14", game::nu::Loader::loadTime(p).getTimestampAsString(), "04-12-201221:04:45");
}
