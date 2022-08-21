/**
  *  \file u/t_server_interface_hostgameclient.cpp
  *  \brief Test for server::interface::HostGameClient
  */

#include "server/interface/hostgameclient.hpp"

#include "t_server_interface.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/except/invaliddataexception.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"

using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Segment;

namespace {
    server::Value_t* makeGameInfo()
    {
        // A (partial) schedule
        Hash::Ref_t sch = Hash::create();
        sch->setNew("type",      server::makeIntegerValue(1));        // WEEKLY
        sch->setNew("weekdays",  server::makeIntegerValue(19));
        sch->setNew("interval",  server::makeIntegerValue(6));
        sch->setNew("daytime",   server::makeIntegerValue(1400));

        // A game
        Hash::Ref_t h = Hash::create();
        h->setNew("id",                  server::makeIntegerValue(43));
        h->setNew("state",               server::makeStringValue("joining"));
        h->setNew("type",                server::makeStringValue("public"));
        h->setNew("name",                server::makeStringValue("The Name"));
        h->setNew("description",         server::makeStringValue("A test game"));
        h->setNew("difficulty",          server::makeIntegerValue(133));
        h->setNew("currentSchedule",     new HashValue(sch));
        h->setNew("slots",               new VectorValue(Vector::create(Segment().pushBackString("open").pushBackString("occupied").pushBackString("self"))));
        h->setNew("turns",               new VectorValue(Vector::create(Segment().pushBackInteger(0).pushBackInteger(2).pushBackInteger(1))));
        h->setNew("joinable",            server::makeIntegerValue(0));
        h->setNew("userPlays",           server::makeIntegerValue(1));
        h->setNew("scores",              new VectorValue(Vector::create(Segment().pushBackInteger(130).pushBackInteger(140).pushBackInteger(135))));
        h->setNew("scoreName",           server::makeStringValue("test"));
        h->setNew("scoreDescription",    server::makeStringValue("Test Score"));
        h->setNew("host",                server::makeStringValue("thost"));
        h->setNew("hostDescription",     server::makeStringValue("Tim Host"));
        h->setNew("hostKind",            server::makeStringValue("th"));
        h->setNew("shiplist",            server::makeStringValue("plist2"));
        h->setNew("shiplistDescription", server::makeStringValue("PList 2"));
        h->setNew("shiplistKind",        server::makeStringValue("plist"));
        h->setNew("master",              server::makeStringValue("xmaster"));
        h->setNew("masterDescription",   server::makeStringValue("Master X"));
        h->setNew("masterKind",          server::makeStringValue("mak"));
        h->setNew("turn",                server::makeIntegerValue(2));
        h->setNew("lastHostTime",        server::makeIntegerValue(15354520));
        h->setNew("nextHostTime",        server::makeIntegerValue(15356789));
        h->setNew("forum",               server::makeIntegerValue(65));
        h->setNew("userRank",            server::makeIntegerValue(3));
        h->setNew("otherRank",           server::makeIntegerValue(7));

        return new HashValue(h);
    }
}


/** Test simple commands. */
void
TestServerInterfaceHostGameClient::testIt()
{
    using server::interface::HostGame;
    afl::test::CommandHandler mock("testIt");
    server::interface::HostGameClient testee(mock);

    // NEWGAME
    mock.expectCall("NEWGAME");
    mock.provideNewResult(server::makeIntegerValue(12));
    TS_ASSERT_EQUALS(testee.createNewGame(), 12);

    // CLONEGAME
    mock.expectCall("CLONEGAME, 2");
    mock.provideNewResult(server::makeIntegerValue(9));
    TS_ASSERT_EQUALS(testee.cloneGame(2, afl::base::Nothing), 9);
    mock.expectCall("CLONEGAME, 7, joining");
    mock.provideNewResult(server::makeIntegerValue(10));
    TS_ASSERT_EQUALS(testee.cloneGame(7, HostGame::Joining), 10);

    // GAMESETTYPE
    mock.expectCall("GAMESETTYPE, 10, unlisted");
    mock.provideNewResult(0);
    testee.setType(10, HostGame::UnlistedGame);

    // GAMESETSTATE
    mock.expectCall("GAMESETSTATE, 10, running");
    mock.provideNewResult(0);
    testee.setState(10, HostGame::Running);

    // GAMESETOWNER
    mock.expectCall("GAMESETOWNER, 7, 1001");
    mock.provideNewResult(0);
    testee.setOwner(7, "1001");

    // GAMESETNAME
    mock.expectCall("GAMESETNAME, 5, Game Five");
    mock.provideNewResult(0);
    testee.setName(5, "Game Five");

    // GAMELIST ID
    {
        afl::data::IntegerList_t result;
        mock.expectCall("GAMELIST, ID");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(2).pushBackInteger(3).pushBackInteger(5))));
        HostGame::Filter filter;
        filter.requiredState = afl::base::Nothing;
        filter.requiredType = afl::base::Nothing;
        filter.requiredUser = afl::base::Nothing;
        TS_ASSERT_THROWS_NOTHING(testee.getGames(HostGame::Filter(), result));
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], 2);
        TS_ASSERT_EQUALS(result[1], 3);
        TS_ASSERT_EQUALS(result[2], 5);
    }
    {
        afl::data::IntegerList_t result;
        mock.expectCall("GAMELIST, STATE, running, ID");
        mock.provideNewResult(new VectorValue(Vector::create()));
        HostGame::Filter filter;
        filter.requiredState = HostGame::Running;
        TS_ASSERT_THROWS_NOTHING(testee.getGames(filter, result));
        TS_ASSERT_EQUALS(result.size(), 0U);
    }
    {
        afl::data::IntegerList_t result;
        mock.expectCall("GAMELIST, TYPE, public, ID");
        mock.provideNewResult(new VectorValue(Vector::create()));
        HostGame::Filter filter;
        filter.requiredType = HostGame::PublicGame;
        TS_ASSERT_THROWS_NOTHING(testee.getGames(filter, result));
        TS_ASSERT_EQUALS(result.size(), 0U);
    }
    {
        afl::data::IntegerList_t result;
        mock.expectCall("GAMELIST, USER, 1030, ID");
        mock.provideNewResult(new VectorValue(Vector::create()));
        HostGame::Filter filter;
        filter.requiredUser = String_t("1030");
        TS_ASSERT_THROWS_NOTHING(testee.getGames(filter, result));
        TS_ASSERT_EQUALS(result.size(), 0U);
    }
    {
        afl::data::IntegerList_t result;
        mock.expectCall("GAMELIST, STATE, joining, TYPE, unlisted, USER, 1015, ID");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(42))));
        HostGame::Filter filter;
        filter.requiredState = HostGame::Joining;
        filter.requiredType = HostGame::UnlistedGame;
        filter.requiredUser = String_t("1015");
        TS_ASSERT_THROWS_NOTHING(testee.getGames(filter, result));
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0], 42);
    }

    // GAMESET
    {
        afl::data::StringList_t kv;
        kv.push_back("master");
        kv.push_back("zeus");
        kv.push_back("host");
        kv.push_back("phost2");
        mock.expectCall("GAMESET, 8, master, zeus, host, phost2");
        mock.provideNewResult(0);
        TS_ASSERT_THROWS_NOTHING(testee.setConfig(8, kv));
    }

    // GAMEGET
    mock.expectCall("GAMEGET, 7, master");
    mock.provideNewResult(server::makeStringValue("pmaster"));
    TS_ASSERT_EQUALS(testee.getConfig(7, "master"), "pmaster");

    // GAMEMGET [FIXME: needed?]
    {
        afl::data::StringList_t keys;
        keys.push_back("k1");
        keys.push_back("k2");

        afl::data::StringList_t values;

        mock.expectCall("GAMEMGET, 6, k1, k2");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackString("first").pushBackString("second"))));
        TS_ASSERT_THROWS_NOTHING(testee.getConfig(6, keys, values));
        TS_ASSERT_EQUALS(values.size(), 2U);
        TS_ASSERT_EQUALS(values[0], "first");
        TS_ASSERT_EQUALS(values[1], "second");
    }

    // GAMEGETCC
    mock.expectCall("GAMEGETCC, 19, difficulty");
    mock.provideNewResult(server::makeIntegerValue(99));
    TS_ASSERT_EQUALS(testee.getComputedValue(19, "difficulty"), "99");

    // GAMEGETSTATE
    mock.expectCall("GAMEGETSTATE, 1");
    mock.provideNewResult(server::makeStringValue("joining"));
    TS_ASSERT_EQUALS(testee.getState(1), HostGame::Joining);

    // GAMEGETTYPE
    mock.expectCall("GAMEGETTYPE, 18");
    mock.provideNewResult(server::makeStringValue("private"));
    TS_ASSERT_EQUALS(testee.getType(18), HostGame::PrivateGame);

    // GAMEGETOWNER
    mock.expectCall("GAMEGETOWNER, 65");
    mock.provideNewResult(server::makeStringValue("1106"));
    TS_ASSERT_EQUALS(testee.getOwner(65), "1106");

    // GAMEGETNAME
    mock.expectCall("GAMEGETNAME, 8");
    mock.provideNewResult(server::makeStringValue("Eight"));
    TS_ASSERT_EQUALS(testee.getName(8), "Eight");

    // GAMEGETDIR
    mock.expectCall("GAMEGETDIR, 7");
    mock.provideNewResult(server::makeStringValue("g/777"));
    TS_ASSERT_EQUALS(testee.getDirectory(7), "g/777");

    // GAMECHECKPERM
    mock.expectCall("GAMECHECKPERM, 9, anon");
    mock.provideNewResult(server::makeIntegerValue(5));
    TS_ASSERT_EQUALS(testee.getPermissions(9, "anon"), (HostGame::Permissions_t() + HostGame::UserIsOwner + HostGame::UserIsActive));

    // GAMEADDTOOL
    mock.expectCall("GAMEADDTOOL, 3, explmap");
    mock.provideNewResult(server::makeIntegerValue(0));
    TS_ASSERT_EQUALS(testee.addTool(3, "explmap"), false);

    // GAMERMTOOL
    mock.expectCall("GAMERMTOOL, 3, wrap");
    mock.provideNewResult(server::makeIntegerValue(1));
    TS_ASSERT_EQUALS(testee.removeTool(3, "wrap"), true);

    // GAMETOTALS
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("joining", server::makeIntegerValue(12));
        h->setNew("running", server::makeIntegerValue(105));
        h->setNew("finished", server::makeIntegerValue(230));
        mock.expectCall("GAMETOTALS");
        mock.provideNewResult(new HashValue(h));

        HostGame::Totals t = testee.getTotals();
        TS_ASSERT_EQUALS(t.numJoiningGames, 12);
        TS_ASSERT_EQUALS(t.numRunningGames, 105);
        TS_ASSERT_EQUALS(t.numFinishedGames, 230);
    }

    // GAMEUPDATE [FIXME: needed?]
    {
        afl::data::IntegerList_t ids;
        ids.push_back(32);
        ids.push_back(16);
        ids.push_back(8);
        mock.expectCall("GAMEUPDATE, 32, 16, 8");
        mock.provideNewResult(0);
        TS_ASSERT_THROWS_NOTHING(testee.updateGames(ids));
    }

    // GAMERESET
    mock.expectCall("GAMERESET, 55, 13");
    mock.provideNewResult(0);   // does not matter
    TS_ASSERT_THROWS_NOTHING(testee.resetToTurn(55, 13));

    mock.checkFinish();
}

/** Test GAMESTAT/GAMELIST. */
void
TestServerInterfaceHostGameClient::testStat()
{
    using server::interface::HostGame;
    using server::interface::HostSchedule;
    afl::test::CommandHandler mock("testStat");
    server::interface::HostGameClient testee(mock);

    // Minimum answer from GAMESTAT
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("id", server::makeIntegerValue(7));
        h->setNew("state", server::makeStringValue("running"));
        h->setNew("type",  server::makeStringValue("private"));
        mock.expectCall("GAMESTAT, 7");
        mock.provideNewResult(new HashValue(h));
        HostGame::Info i = testee.getInfo(7);

        TS_ASSERT_EQUALS(i.gameId, 7);
        TS_ASSERT_EQUALS(i.state, HostGame::Running);
        TS_ASSERT_EQUALS(i.type, HostGame::PrivateGame);
        TS_ASSERT_EQUALS(i.name, "");
        TS_ASSERT(!i.description.isValid());
        TS_ASSERT(!i.currentSchedule.isValid());
        TS_ASSERT(!i.scoreName.isValid());
        TS_ASSERT(!i.masterName.isValid());
    }

    // Full answer from GAMESTAT
    {
        mock.expectCall("GAMESTAT, 42");
        mock.provideNewResult(makeGameInfo());
        HostGame::Info i = testee.getInfo(42);

        TS_ASSERT_EQUALS(i.gameId, 43); // value as provided by mock, deliberately different from parameter
        TS_ASSERT_EQUALS(i.state, HostGame::Joining);
        TS_ASSERT_EQUALS(i.type, HostGame::PublicGame);
        TS_ASSERT_EQUALS(i.name, "The Name");
        TS_ASSERT(i.description.isSame(String_t("A test game")));
        TS_ASSERT_EQUALS(i.difficulty, 133);

        TS_ASSERT(i.currentSchedule.isValid());
        TS_ASSERT(i.currentSchedule.get()->type.isSame(HostSchedule::Weekly));

        TS_ASSERT(i.slotStates.isValid());
        TS_ASSERT_EQUALS(i.slotStates.get()->size(), 3U);
        TS_ASSERT_EQUALS((*i.slotStates.get())[0], HostGame::OpenSlot);
        TS_ASSERT_EQUALS((*i.slotStates.get())[1], HostGame::OccupiedSlot);
        TS_ASSERT_EQUALS((*i.slotStates.get())[2], HostGame::SelfSlot);

        TS_ASSERT(i.turnStates.isValid());
        TS_ASSERT_EQUALS(i.turnStates.get()->size(), 3U);
        TS_ASSERT_EQUALS((*i.turnStates.get())[0], 0);
        TS_ASSERT_EQUALS((*i.turnStates.get())[1], 2);
        TS_ASSERT_EQUALS((*i.turnStates.get())[2], 1);

        TS_ASSERT(i.joinable.isSame(false));
        TS_ASSERT(i.userPlays.isSame(true));

        TS_ASSERT(i.scores.isValid());
        TS_ASSERT_EQUALS(i.scores.get()->size(), 3U);
        TS_ASSERT_EQUALS((*i.scores.get())[0], 130);
        TS_ASSERT_EQUALS((*i.scores.get())[1], 140);
        TS_ASSERT_EQUALS((*i.scores.get())[2], 135);

        TS_ASSERT(i.scoreName.isSame(String_t("test")));
        TS_ASSERT(i.scoreDescription.isSame(String_t("Test Score")));
        TS_ASSERT_EQUALS(i.hostName, "thost");
        TS_ASSERT_EQUALS(i.hostDescription, "Tim Host");
        TS_ASSERT_EQUALS(i.hostKind, "th");
        TS_ASSERT_EQUALS(i.shipListName, "plist2");
        TS_ASSERT_EQUALS(i.shipListDescription, "PList 2");
        TS_ASSERT_EQUALS(i.shipListKind, "plist");
        TS_ASSERT(i.masterName.isSame(String_t("xmaster")));
        TS_ASSERT(i.masterDescription.isSame(String_t("Master X")));
        TS_ASSERT(i.masterKind.isSame(String_t("mak")));

        TS_ASSERT_EQUALS(i.turnNumber, 2);
        TS_ASSERT(i.lastHostTime.isSame(15354520));
        TS_ASSERT(i.nextHostTime.isSame(15356789));
        TS_ASSERT(i.forumId.isSame(65));
        TS_ASSERT(i.userRank.isSame(3));
        TS_ASSERT(i.otherRank.isSame(7));
    }

    // Full answer from GAMELIST
    {
        mock.expectCall("GAMELIST");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackNew(makeGameInfo()))));
        std::vector<HostGame::Info> infos;
        testee.getInfos(HostGame::Filter(), false, infos);
        TS_ASSERT_EQUALS(infos.size(), 1U);
        TS_ASSERT_EQUALS(infos[0].gameId, 43);
        TS_ASSERT_EQUALS(infos[0].state, HostGame::Joining);
    }

    // Variations of GAMELIST
    {
        std::vector<HostGame::Info> infos;

        mock.expectCall("GAMELIST, VERBOSE");
        mock.provideNewResult(0);
        testee.getInfos(HostGame::Filter(), true, infos);

        mock.expectCall("GAMELIST, STATE, running");
        mock.provideNewResult(0);
        HostGame::Filter f1;
        f1.requiredState = HostGame::Running;
        testee.getInfos(f1, false, infos);

        mock.expectCall("GAMELIST, TYPE, unlisted");
        mock.provideNewResult(0);
        HostGame::Filter f2;
        f2.requiredType = HostGame::UnlistedGame;
        testee.getInfos(f2, false, infos);

        mock.expectCall("GAMELIST, USER, u32");
        mock.provideNewResult(0);
        HostGame::Filter f3;
        f3.requiredUser = String_t("u32");
        testee.getInfos(f3, false, infos);

        mock.expectCall("GAMELIST, STATE, joining, TYPE, public, USER, 1003, VERBOSE");
        mock.provideNewResult(0);
        HostGame::Filter f4;
        f4.requiredState = HostGame::Joining;
        f4.requiredType = HostGame::PublicGame;
        f4.requiredUser = String_t("1003");
        testee.getInfos(f4, true, infos);

        mock.expectCall("GAMELIST, USER, 1003, HOST, qhost, TOOL, multitool, SHIPLIST, list, MASTER, pmaster, VERBOSE");
        mock.provideNewResult(0);
        HostGame::Filter f5;
        f5.requiredUser = String_t("1003");
        f5.requiredHost = String_t("qhost");
        f5.requiredTool = String_t("multitool");
        f5.requiredShipList = String_t("list");
        f5.requiredMaster = String_t("pmaster");
        testee.getInfos(f5, true, infos);

        TS_ASSERT_EQUALS(infos.size(), 0U);
    }

    mock.checkFinish();
}

/** Test GAMELSTOOLS. */
void
TestServerInterfaceHostGameClient::testTools()
{
    using server::interface::HostGame;
    using server::interface::HostTool;
    afl::test::CommandHandler mock("testTools");
    server::interface::HostGameClient testee(mock);

    // Empty answer
    {
        mock.expectCall("GAMELSTOOLS, 12");
        mock.provideNewResult(new VectorValue(Vector::create()));
        std::vector<HostTool::Info> infos;
        TS_ASSERT_THROWS_NOTHING(testee.getTools(12, infos));
        TS_ASSERT_EQUALS(infos.size(), 0U);
    }

    // Nonempty answer
    {
        Hash::Ref_t a = Hash::create();
        a->setNew("id",           server::makeStringValue("a"));
        a->setNew("description",  server::makeStringValue("apple"));
        a->setNew("kind",         server::makeStringValue("fruit"));
        a->setNew("default",      server::makeIntegerValue(0));

        Hash::Ref_t b = Hash::create();
        b->setNew("id",           server::makeStringValue("b"));
        b->setNew("description",  server::makeStringValue("bread"));
        b->setNew("kind",         server::makeStringValue("staple"));
        b->setNew("default",      server::makeIntegerValue(1));

        Vector::Ref_t v = Vector::create();
        v->pushBackNew(new HashValue(a));
        v->pushBackNew(0);
        v->pushBackNew(new HashValue(b));

        mock.expectCall("GAMELSTOOLS, 39");
        mock.provideNewResult(new VectorValue(v));
        std::vector<HostTool::Info> infos;
        TS_ASSERT_THROWS_NOTHING(testee.getTools(39, infos));
        TS_ASSERT_EQUALS(infos.size(), 3U);

        // First
        TS_ASSERT_EQUALS(infos[0].id, "a");
        TS_ASSERT_EQUALS(infos[0].description, "apple");
        TS_ASSERT_EQUALS(infos[0].kind, "fruit");
        TS_ASSERT_EQUALS(infos[0].isDefault, false);

        // Second, default deserialisation for missing members
        TS_ASSERT_EQUALS(infos[1].id, "");
        TS_ASSERT_EQUALS(infos[1].description, "");
        TS_ASSERT_EQUALS(infos[1].kind, "");
        TS_ASSERT_EQUALS(infos[1].isDefault, false);

        // Last
        TS_ASSERT_EQUALS(infos[2].id, "b");
        TS_ASSERT_EQUALS(infos[2].description, "bread");
        TS_ASSERT_EQUALS(infos[2].kind, "staple");
        TS_ASSERT_EQUALS(infos[2].isDefault, true);
    }

    mock.checkFinish();
}

/** Test GAMEGETVC. */
void
TestServerInterfaceHostGameClient::testVC()
{
    using server::interface::HostGame;
    afl::test::CommandHandler mock("testVC");
    server::interface::HostGameClient testee(mock);

    // Null answer
    {
        mock.expectCall("GAMEGETVC, 89");
        mock.provideNewResult(0);
        HostGame::VictoryCondition vc = testee.getVictoryCondition(89);

        TS_ASSERT_EQUALS(vc.endCondition, String_t());
        TS_ASSERT(!vc.endTurn.isValid());
        TS_ASSERT(!vc.endProbability.isValid());
        TS_ASSERT(!vc.endScore.isValid());
        TS_ASSERT(!vc.endScoreName.isValid());
        TS_ASSERT(!vc.endScoreDescription.isValid());
        TS_ASSERT(!vc.referee.isValid());
        TS_ASSERT(!vc.refereeDescription.isValid());
    }

    // Full answer
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("endCondition",        server::makeStringValue("turn"));
        h->setNew("endTurn",             server::makeIntegerValue(90));
        h->setNew("endProbability",      server::makeIntegerValue(5));
        h->setNew("endScore",            server::makeIntegerValue(15000));
        h->setNew("endScoreName",        server::makeStringValue("Ultra Score"));
        h->setNew("endScoreDescription", server::makeStringValue("Best Ever!"));
        h->setNew("referee",             server::makeStringValue("Bibi"));
        h->setNew("refereeDescription",  server::makeStringValue("Info..."));
        mock.expectCall("GAMEGETVC, 76");
        mock.provideNewResult(new HashValue(h));

        HostGame::VictoryCondition vc = testee.getVictoryCondition(76);

        TS_ASSERT_EQUALS(vc.endCondition, "turn");
        TS_ASSERT(vc.endTurn.isSame(90));
        TS_ASSERT(vc.endProbability.isSame(5));
        TS_ASSERT(vc.endScore.isSame(15000));
        TS_ASSERT(vc.endScoreName.isSame(String_t("Ultra Score")));
        TS_ASSERT(vc.endScoreDescription.isSame(String_t("Best Ever!")));
        TS_ASSERT(vc.referee.isSame(String_t("Bibi")));
        TS_ASSERT(vc.refereeDescription.isSame(String_t("Info...")));
    }

    mock.checkFinish();
}

/** Test errors. */
void
TestServerInterfaceHostGameClient::testErrors()
{
    using server::interface::HostGame;
    afl::test::CommandHandler mock("testErrors");
    server::interface::HostGameClient testee(mock);

    // GAMEGETSTATE
    mock.expectCall("GAMEGETSTATE, 7");
    mock.provideNewResult(server::makeStringValue("thinking"));
    TS_ASSERT_THROWS(testee.getState(7), afl::except::InvalidDataException);

    // GAMEGETTYPE
    mock.expectCall("GAMEGETTYPE, 12");
    mock.provideNewResult(server::makeStringValue("fun"));
    TS_ASSERT_THROWS(testee.getType(12), afl::except::InvalidDataException);

    // GAMESTAT with empty result (means: state/type don't decode)
    mock.expectCall("GAMESTAT, 9");
    mock.provideNewResult(0);
    TS_ASSERT_THROWS(testee.getInfo(9), afl::except::InvalidDataException);

    // GAMESTAT with invalid state
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("id",                  server::makeIntegerValue(43));
        h->setNew("state",               server::makeStringValue("fighting"));
        h->setNew("type",                server::makeStringValue("public"));
        mock.expectCall("GAMESTAT, 2");
        mock.provideNewResult(new HashValue(h));
        TS_ASSERT_THROWS(testee.getInfo(2), afl::except::InvalidDataException);
    }

    // GAMESTAT with invalid type
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("id",                  server::makeIntegerValue(43));
        h->setNew("state",               server::makeStringValue("finished"));
        h->setNew("type",                server::makeStringValue("boring"));
        mock.expectCall("GAMESTAT, 3");
        mock.provideNewResult(new HashValue(h));
        TS_ASSERT_THROWS(testee.getInfo(3), afl::except::InvalidDataException);
    }

    // GAMESTAT with invalid slot state
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("id",                  server::makeIntegerValue(43));
        h->setNew("state",               server::makeStringValue("finished"));
        h->setNew("type",                server::makeStringValue("public"));
        h->setNew("slots", new VectorValue(Vector::create(Segment().pushBackString("meh"))));
        mock.expectCall("GAMESTAT, 4");
        mock.provideNewResult(new HashValue(h));
        TS_ASSERT_THROWS(testee.getInfo(4), afl::except::InvalidDataException);
    }
}

