/**
  *  \file test/game/interface/playerpropertytest.cpp
  *  \brief Test for game::interface::PlayerProperty
  */

#include "game/interface/playerproperty.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/game.hpp"
#include "game/score/scoreid.hpp"
#include "game/score/turnscore.hpp"
#include "game/score/turnscorelist.hpp"
#include "game/turn.hpp"
#include "interpreter/test/valueverifier.hpp"

using game::score::TurnScore;
using game::score::TurnScoreList;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

namespace {
    void populateScores(TurnScoreList& scores, int turnNr, int playerNr)
    {
        TurnScore& t = scores.addTurn(turnNr, game::Timestamp());
        TurnScoreList::Slot_t pSlot = scores.addSlot(game::score::ScoreId_Planets);
        TurnScoreList::Slot_t bSlot = scores.addSlot(game::score::ScoreId_Bases);
        TurnScoreList::Slot_t cSlot = scores.addSlot(game::score::ScoreId_Capital);
        TurnScoreList::Slot_t fSlot = scores.addSlot(game::score::ScoreId_Freighters);
        TurnScoreList::Slot_t qSlot = scores.addSlot(game::score::ScoreId_BuildPoints);

        // Scores for primary player
        t.set(pSlot, playerNr, 10);
        t.set(bSlot, playerNr, 20);
        t.set(cSlot, playerNr, 30);
        t.set(fSlot, playerNr, 40);
        t.set(qSlot, playerNr, 50);

        // Scores for other player, to exercise "Totals"
        t.set(pSlot, playerNr+1, 1);
        t.set(bSlot, playerNr+1, 2);
        t.set(cSlot, playerNr+1, 3);
        t.set(fSlot, playerNr+1, 4);
        t.set(qSlot, playerNr+1, 5);
    }
}

AFL_TEST("game.interface.PlayerProperty", a)
{
    // Player List
    const int PLAYER_NR = 3;
    game::PlayerList pl;
    game::Player* p = pl.create(PLAYER_NR);
    p->setName(game::Player::ShortName, "Shortie");
    p->setName(game::Player::LongName, "Longie");
    p->setName(game::Player::AdjectiveName, "Addie");

    // Game
    const int TURN_NR = 12;
    game::Game g;
    g.currentTurn().setTurnNumber(TURN_NR);
    populateScores(g.scores(), TURN_NR, PLAYER_NR);

    g.teamSettings().setPlayerTeam(PLAYER_NR, 7);

    // Host configuration
    game::config::HostConfiguration config;
    config[game::config::HostConfiguration::PlayerRace].set("11,10,9,8,7,6,5,4,3,2,1");
    config[game::config::HostConfiguration::PlayerSpecialMission].set("3,2,1,6,5,4,9,8,7,11,10");

    // Translator
    afl::string::NullTranslator tx;

    // Test
    verifyNewString (a("iplAdjName"),         getPlayerProperty(PLAYER_NR, game::interface::iplAdjName,         pl, g, config, tx), "Addie");
    verifyNewString (a("iplFullName"),        getPlayerProperty(PLAYER_NR, game::interface::iplFullName,        pl, g, config, tx), "Longie");
    verifyNewInteger(a("iplId"),              getPlayerProperty(PLAYER_NR, game::interface::iplId,              pl, g, config, tx), PLAYER_NR);
    verifyNewInteger(a("iplMission"),         getPlayerProperty(PLAYER_NR, game::interface::iplMission,         pl, g, config, tx), 1);
    verifyNewInteger(a("iplPBPs"),            getPlayerProperty(PLAYER_NR, game::interface::iplPBPs,            pl, g, config, tx), 50);
    verifyNewInteger(a("iplRaceId"),          getPlayerProperty(PLAYER_NR, game::interface::iplRaceId,          pl, g, config, tx), 9);
    verifyNewInteger(a("iplTeam"),            getPlayerProperty(PLAYER_NR, game::interface::iplTeam,            pl, g, config, tx), 7);
    verifyNewString (a("iplShortName"),       getPlayerProperty(PLAYER_NR, game::interface::iplShortName,       pl, g, config, tx), "Shortie");
    verifyNewInteger(a("iplScoreBases"),      getPlayerProperty(PLAYER_NR, game::interface::iplScoreBases,      pl, g, config, tx), 20);
    verifyNewInteger(a("iplScorePlanets"),    getPlayerProperty(PLAYER_NR, game::interface::iplScorePlanets,    pl, g, config, tx), 10);
    verifyNewInteger(a("iplScoreShips"),      getPlayerProperty(PLAYER_NR, game::interface::iplScoreShips,      pl, g, config, tx), 70);
    verifyNewInteger(a("iplScore"),           getPlayerProperty(PLAYER_NR, game::interface::iplScore,           pl, g, config, tx), 2840);
    verifyNewInteger(a("iplScoreCapital"),    getPlayerProperty(PLAYER_NR, game::interface::iplScoreCapital,    pl, g, config, tx), 30);
    verifyNewInteger(a("iplScoreFreighters"), getPlayerProperty(PLAYER_NR, game::interface::iplScoreFreighters, pl, g, config, tx), 40);
    verifyNewInteger(a("iplTotalShips"),      getPlayerProperty(PLAYER_NR, game::interface::iplTotalShips,      pl, g, config, tx), 77);
    verifyNewInteger(a("iplTotalFreighters"), getPlayerProperty(PLAYER_NR, game::interface::iplTotalFreighters, pl, g, config, tx), 44);
    verifyNewInteger(a("iplTotalCapital"),    getPlayerProperty(PLAYER_NR, game::interface::iplTotalCapital,    pl, g, config, tx), 33);

    // Out-of-range Id can be accessed for Ids >= 0
    verifyNewInteger(a("iplId 99"),           getPlayerProperty(99, game::interface::iplId,      pl, g, config, tx), 99);
    verifyNewInteger(a("iplId 0"),            getPlayerProperty(0,  game::interface::iplId,      pl, g, config, tx), 0);
    verifyNewNull   (a("iplId -1"),           getPlayerProperty(-1, game::interface::iplId,      pl, g, config, tx));

    // Other properties null for nonexistant slots
    verifyNewNull   (a("iplScore 99"),        getPlayerProperty(99, game::interface::iplScore,   pl, g, config, tx));
    verifyNewNull   (a("iplScore 0"),         getPlayerProperty(0,  game::interface::iplScore,   pl, g, config, tx));
    verifyNewNull   (a("iplAdjName 99"),      getPlayerProperty(99, game::interface::iplAdjName, pl, g, config, tx));
    verifyNewNull   (a("iplAdjName 0"),       getPlayerProperty(0,  game::interface::iplAdjName, pl, g, config, tx));
}
