/**
  *  \file test/server/play/mainpackertest.cpp
  *  \brief Test for server::play::MainPacker
  */

#include "server/play/mainpacker.hpp"

#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/registrationkey.hpp"
#include "game/score/scoreid.hpp"
#include "game/score/turnscore.hpp"
#include "game/score/turnscorelist.hpp"
#include "game/spec/shiplist.hpp"
#include "game/task.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "game/vcr/test/database.hpp"

using afl::base::Ref;
using game::Game;
using game::HostVersion;
using game::Player;
using game::PlayerSet_t;
using game::RegistrationKey;
using game::Root;
using game::Session;
using game::StatusTask_t;
using game::Task_t;
using game::Turn;
using game::config::HostConfiguration;
using game::score::TurnScore;
using game::spec::ShipList;
using game::vcr::test::Database;

namespace {
    class MyTurnLoader : public game::TurnLoader {
        virtual PlayerStatusSet_t getPlayerStatus(int /*player*/, String_t& /*extra*/, afl::string::Translator& /*tx*/) const
            { return PlayerStatusSet_t(); }
        virtual std::auto_ptr<Task_t> loadCurrentTurn(Game& /*game*/, int /*player*/, Root& /*root*/, Session& /*session*/, std::auto_ptr<StatusTask_t> then)
            { return game::makeConfirmationTask(true, then); }
        virtual std::auto_ptr<Task_t> saveCurrentTurn(const Game& /*game*/, PlayerSet_t /*players*/, SaveOptions_t /*opts*/, const Root& /*root*/, Session& /*session*/, std::auto_ptr<StatusTask_t> then)
            { return game::makeConfirmationTask(true, then); }
        virtual void getHistoryStatus(int /*player*/, int /*turn*/, afl::base::Memory<HistoryStatus> /*status*/, const Root& /*root*/)
            { }
        virtual std::auto_ptr<Task_t> loadHistoryTurn(Turn& /*turn*/, Game& /*game*/, int /*player*/, int /*turnNumber*/, Root& /*root*/, Session& /*session*/, std::auto_ptr<StatusTask_t> then)
            { return game::makeConfirmationTask(true, then); }
        virtual std::auto_ptr<Task_t> saveConfiguration(const Root& /*root*/, afl::sys::LogListener& /*log*/, afl::string::Translator& /*tx*/, std::auto_ptr<Task_t> then)
            { return then; }
        virtual String_t getProperty(Property p)
            {
                switch (p) {
                 case LocalFileFormatProperty: return "local fmt";
                 case RemoteFileFormatProperty: return "remote fmt";
                 case RootDirectoryProperty: return "/root";
                }
                return String_t();
            }
    };
}

AFL_TEST("server.play.MainPacker", a)
{
    // Session
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    Session session(tx, fs);

    // Game
    Ref<Game> g(*new Game());
    session.setGame(g.asPtr());
    g->teamSettings().setPlayerTeam(1, 5);
    g->currentTurn().setTurnNumber(42);
    g->currentTurn().setTimestamp(game::Timestamp(2004, 12, 25, 13, 35, 40));
    g->setViewpointPlayer(4);

    // - Scores
    TurnScore& s = g->scores().addTurn(42, game::Timestamp());
    s.set(g->scores().addSlot(game::score::ScoreId_Planets),     4, 50);
    s.set(g->scores().addSlot(game::score::ScoreId_Bases),       4, 20);
    s.set(g->scores().addSlot(game::score::ScoreId_Freighters),  4, 30);
    s.set(g->scores().addSlot(game::score::ScoreId_Capital),     4, 40);
    s.set(g->scores().addSlot(game::score::ScoreId_BuildPoints), 4, 99);

    // - Messages
    for (int i = 0; i < 10; ++i) {
        g->currentTurn().inbox().addMessage("text", 42);
    }
    for (int i = 0; i < 5; ++i) {
        g->currentTurn().outbox().addMessage(4, "text", game::PlayerSet_t(1));
    }

    // - VCRs
    Ref<Database> db(*new Database());
    for (int i = 0; i < 7; ++i) {
        db->addBattle();
    }
    g->currentTurn().setBattles(db.asPtr());

    // Root
    Ref<Root> r(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,1,3)), RegistrationKey::Unregistered));
    session.setRoot(r.asPtr());

    // - Turn Loader
    r->setTurnLoader(new MyTurnLoader());

    // - Player 4
    Player& p4 = *r->playerList().create(4);
    p4.setName(Player::ShortName, "Four Short");
    p4.setName(Player::LongName, "The Fourth Long Name");
    p4.setName(Player::AdjectiveName, "fourish");

    // - Host config
    r->hostConfiguration()[HostConfiguration::PlayerRace].set("3,4,5,6,7");
    r->hostConfiguration()[HostConfiguration::PlayerSpecialMission].set("10,9,8,7,6,5");

    // Ship list
    Ref<ShipList> sl(*new ShipList());
    session.setShipList(sl.asPtr());
    sl->hulls().create(70);

    // Properties
    server::play::getSessionProperties(session)["k1"] = "v1";

    // Test it
    server::play::MainPacker testee(session);
    a.checkEqual("01. getName", testee.getName(), "main");

    // Verify data content
    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    a.checkEqual("11. MY.INMSGS",           ap("MY.INMSGS").toInteger(),          10);
    a.checkEqual("12. MY.OUTMSGS",          ap("MY.OUTMSGS").toInteger(),         5);
    a.checkEqual("13. MY.RACE",             ap("MY.RACE").toInteger(),            4);
    a.checkEqual("14. MY.RACE.ID",          ap("MY.RACE.ID").toInteger(),         6);
    a.checkEqual("15. MY.RACE.MISSION",     ap("MY.RACE.MISSION").toInteger(),    7);
    a.checkEqual("16. MY.VCRS",             ap("MY.VCRS").toInteger(),            7);
    a.checkEqual("17. SYSTEM.GAMETYPE$",    ap("SYSTEM.GAMETYPE").toInteger(),    1);
    a.checkEqual("18. SYSTEM.LOCAL",        ap("SYSTEM.LOCAL").toString(),        "local fmt");
    a.checkEqual("19. SYSTEM.HOST",         ap("SYSTEM.HOST").toString(),         "PHost");
    a.checkEqual("20. SYSTEM.HOST$",        ap("SYSTEM.HOST$").toInteger(),       2);
    a.checkEqual("21. SYSTEM.HOSTVERSION",  ap("SYSTEM.HOSTVERSION").toInteger(), 401003);
    a.checkDifferent("22. SYSTEM.REGSTR1",  ap("SYSTEM.REGSTR1").toString(),      "");
    a.checkDifferent("23. SYSTEM.REGSTR2",  ap("SYSTEM.REGSTR2").toString(),      "");
    a.checkEqual("24. SYSTEM.REMOTE",       ap("SYSTEM.REMOTE").toString(),       "remote fmt");
    a.checkDifferent("25. SYSTEM.VERSION",  ap("SYSTEM.VERSION").toString(),      "");
    a.checkDifferent("26. SYSTEM.VERSION$", ap("SYSTEM.VERSION$").toInteger(),    0);
    a.checkEqual("27. TURN",                ap("TURN").toInteger(),               42);
    a.checkEqual("28. TURN.DATE",           ap("TURN.DATE").toString(),           "12-25-2004");
    a.checkEqual("29. TURN.TIME",           ap("TURN.TIME").toString(),           "13:35:40");
    a.checkEqual("30. NUMHULLS",            ap("NUMHULLS").toInteger(),           70);

    a.checkEqual("41. prop", ap("PROP")("k1").toString(), "v1");
}
