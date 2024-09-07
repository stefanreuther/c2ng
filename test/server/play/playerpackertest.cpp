/**
  *  \file test/server/play/playerpackertest.cpp
  *  \brief Test for server::play::PlayerPacker
  */

#include "server/play/playerpacker.hpp"

#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/score/scoreid.hpp"
#include "game/score/turnscore.hpp"
#include "game/score/turnscorelist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"

using afl::base::Ref;
using game::Game;
using game::HostVersion;
using game::Player;
using game::Root;
using game::config::HostConfiguration;
using game::score::TurnScore;

AFL_TEST("server.play.PlayerPacker", a)
{
    // Session
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);

    // Game
    Ref<Game> g(*new Game());
    session.setGame(g.asPtr());
    g->teamSettings().setPlayerTeam(1, 5);
    g->currentTurn().setTurnNumber(42);

    // - Scores
    TurnScore& s = g->scores().addTurn(42, game::Timestamp());
    s.set(g->scores().addSlot(game::score::ScoreId_Planets),     1, 50);
    s.set(g->scores().addSlot(game::score::ScoreId_Bases),       1, 20);
    s.set(g->scores().addSlot(game::score::ScoreId_Freighters),  1, 30);
    s.set(g->scores().addSlot(game::score::ScoreId_Capital),     1, 40);
    s.set(g->scores().addSlot(game::score::ScoreId_BuildPoints), 1, 99);

    // Root
    Ref<Root> r(game::test::makeRoot(HostVersion()));
    session.setRoot(r.asPtr());

    // - Player 1
    Player& p1 = *r->playerList().create(1);
    p1.setName(Player::ShortName, "Shortie");
    p1.setName(Player::LongName, "The Long Name");
    p1.setName(Player::AdjectiveName, "adj");

    // - Player 2 (pseudo)
    Player& p2 = *r->playerList().create(2);
    p2.initAlien();

    // - Host config
    r->hostConfiguration()[HostConfiguration::PlayerRace].set("3,4,5,6,7");
    r->hostConfiguration()[HostConfiguration::PlayerSpecialMission].set("10,9,8,7");

    // Test it
    server::play::PlayerPacker testee(session);
    a.checkEqual("01. getName", testee.getName(), "player");

    // Verify data content
    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    a.checkGreaterEqual("11. getArraySize", ap.getArraySize(), 3U);
    a.check("12. result", ap[0].isNull());
    a.check("13. result", !ap[1].isNull());
    a.check("14. result", ap[2].isNull());

    a.checkEqual("21. bases",      ap[1]("BASES").toInteger(),            20);
    a.checkEqual("22. pbps",       ap[1]("PBPS").toInteger(),             99);
    a.checkEqual("23. planets",    ap[1]("PLANETS").toInteger(),          50);
    a.checkEqual("24. race",       ap[1]("RACE").toString(),              "The Long Name");
    a.checkEqual("25. race$",      ap[1]("RACE$").toInteger(),            1);
    a.checkEqual("26. adj",        ap[1]("RACE.ADJ").toString(),          "adj");
    a.checkEqual("27. id",         ap[1]("RACE.ID").toInteger(),          3);
    a.checkEqual("28. mission",    ap[1]("RACE.MISSION").toInteger(),     10);
    a.checkEqual("29. short",      ap[1]("RACE.SHORT").toString(),        "Shortie");
    a.checkEqual("30. score",      ap[1]("SCORE").toInteger(),            3330);
    a.checkEqual("31. ships",      ap[1]("SHIPS").toInteger(),            70);
    a.checkEqual("32. capital",    ap[1]("SHIPS.CAPITAL").toInteger(),    40);
    a.checkEqual("33. freighters", ap[1]("SHIPS.FREIGHTERS").toInteger(), 30);
    a.checkEqual("34. team",       ap[1]("TEAM").toInteger(),             5);
}
