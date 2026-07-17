/**
  *  \file test/game/v3/baseturnloadertest.cpp
  *  \brief Test for game::v3::BaseTurnLoader
  */

#include "game/v3/baseturnloader.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/task.hpp"
#include "game/test/files.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"

using afl::base::Ref;
using afl::charset::Charset;
using afl::charset::CodepageCharset;
using afl::io::FileSystem;
using afl::io::InternalFileSystem;
using afl::string::NullTranslator;
using afl::sys::InternalEnvironment;
using afl::sys::Log;
using game::Game;
using game::PlayerSet_t;
using game::Root;
using game::Session;
using game::StatusTask_t;
using game::Task_t;
using game::Turn;
using game::config::UserConfiguration;

namespace {
    /* Implementation of BaseTurnLoader populating the missing functions.
       None of the missing functions is supposed to be called. */
    class Testee : public game::v3::BaseTurnLoader {
     public:
        Testee(const afl::base::Ref<afl::io::Directory>& specDir, std::auto_ptr<Charset> charset, FileSystem& fs, util::ProfileDirectory* pProfile)
            : BaseTurnLoader(specDir, charset, fs, pProfile)
            { }
        virtual PlayerStatusSet_t getPlayerStatus(int /*player*/, String_t& /*extra*/, afl::string::Translator& /*tx*/) const
            { throw afl::except::AssertionFailedException("unexpected", "getPlayerStatus"); }
        virtual std::auto_ptr<Task_t> loadCurrentTurn(Game& /*game*/, int /*player*/, Root& /*root*/, Session& /*session*/, std::auto_ptr<StatusTask_t> /*then*/)
            { throw afl::except::AssertionFailedException("unexpected", "loadCurrentTurn"); }
        virtual std::auto_ptr<Task_t> saveCurrentTurn(const Game& /*game*/, PlayerSet_t /*players*/, SaveOptions_t /*opts*/, const Root& /*root*/, Session& /*session*/, std::auto_ptr<StatusTask_t> /*then*/)
            { throw afl::except::AssertionFailedException("unexpected", "saveCurrentTurn"); }
        virtual String_t getProperty(Property /*p*/)
            { throw afl::except::AssertionFailedException("unexpected", "getProperty"); }
    };

    void populateFileSystem(InternalFileSystem& fs)
    {
        fs.createDirectory("/spec");
        fs.createDirectory("/game");
        fs.createDirectory("/profile");
        fs.createDirectory("/backup");
        fs.openFile("/spec/xyplan.dat",    FileSystem::Create)->fullWrite(game::test::getDefaultPlanetCoordinates());
        fs.openFile("/spec/planet.nm",     FileSystem::Create)->fullWrite(game::test::getDefaultPlanetNames());
        fs.openFile("/spec/storm.nm",      FileSystem::Create)->fullWrite(game::test::getDefaultIonStormNames());
        fs.openFile("/backup/result7.002", FileSystem::Create)->fullWrite(game::test::getResultFile35());
    }
}

/** Test history handling functions, success case. */
AFL_TEST("game.v3.BaseTurnLoader:history:success", a)
{
    // File system
    InternalFileSystem fs;
    populateFileSystem(fs);

    // Test object
    Testee testee(fs.openDirectory("/spec"), std::auto_ptr<Charset>(new CodepageCharset(afl::charset::g_codepageLatin1)), fs, 0);

    // Root
    Ref<Root> root = game::test::makeRoot(game::HostVersion());
    root->userConfiguration()[UserConfiguration::Backup_Result].set("/backup/result%p.%t");

    // Test getHistoryStatus
    game::TurnLoader::HistoryStatus status[3];
    testee.getHistoryStatus(7, 1, status, *root);
    a.checkEqual("turn 1 status", status[0], game::TurnLoader::Negative);
    a.checkEqual("turn 2 status", status[1], game::TurnLoader::StronglyPositive);
    a.checkEqual("turn 3 status", status[2], game::TurnLoader::Negative);

    // Test loadHistoryTurn
    NullTranslator tx;
    Session session(tx, fs);
    Ref<Game> g = *new Game();
    Ref<Turn> t = *new Turn();
    bool flag = false;
    testee.loadHistoryTurn(*t, *g, 7, 2, *root, session, game::makeResultTask(flag))
        ->call();
    a.check("loadHistoryTurn status", flag);
    a.checkEqual("turn number", t->getTurnNumber(), 2);
    a.checkEqual("planets owner", t->universe().planets().get(30)->getOwner().orElse(-1), 7);
}

/** Test history handling functions, failure case: non-existant turn. */
AFL_TEST("game.v3.BaseTurnLoader:history:failure", a)
{
    // File system
    InternalFileSystem fs;
    populateFileSystem(fs);

    // Test object
    Testee testee(fs.openDirectory("/spec"), std::auto_ptr<Charset>(new CodepageCharset(afl::charset::g_codepageLatin1)), fs, 0);

    // Root
    Ref<Root> root = game::test::makeRoot(game::HostVersion());
    root->userConfiguration()[UserConfiguration::Backup_Result].set("/backup/result%p.%t");

    // Test loadHistoryTurn for non-existant turn
    NullTranslator tx;
    Session session(tx, fs);
    Ref<Game> g = *new Game();
    Ref<Turn> t = *new Turn();
    bool flag = true;
    testee.loadHistoryTurn(*t, *g, 7, 3, *root, session, game::makeResultTask(flag))
        ->call();
    a.check("loadHistoryTurn status", !flag);
}

/** Test history handling functions, failure case: no backups configured. */
AFL_TEST("game.v3.BaseTurnLoader:history:unconfigured", a)
{
    // File system
    InternalFileSystem fs;
    populateFileSystem(fs);

    // Test object
    Testee testee(fs.openDirectory("/spec"), std::auto_ptr<Charset>(new CodepageCharset(afl::charset::g_codepageLatin1)), fs, 0);

    // Root
    Ref<Root> root = game::test::makeRoot(game::HostVersion());

    // Test getHistoryStatus
    game::TurnLoader::HistoryStatus status[3];
    testee.getHistoryStatus(7, 1, status, *root);
    a.checkEqual("turn 1 status", status[0], game::TurnLoader::Negative);
    a.checkEqual("turn 2 status", status[1], game::TurnLoader::Negative);
    a.checkEqual("turn 3 status", status[2], game::TurnLoader::Negative);

    // Test loadHistoryTurn
    NullTranslator tx;
    Session session(tx, fs);
    Ref<Game> g = *new Game();
    Ref<Turn> t = *new Turn();
    bool flag = true;
    testee.loadHistoryTurn(*t, *g, 7, 2, *root, session, game::makeResultTask(flag))
        ->call();
    a.check("loadHistoryTurn status", !flag);
}

/** Test saveConfiguration(). */
AFL_TEST("game.v3.BaseTurnLoader:saveConfiguration", a)
{
    // File system
    InternalFileSystem fs;
    populateFileSystem(fs);

    // Test object
    Testee testee(fs.openDirectory("/spec"), std::auto_ptr<Charset>(new CodepageCharset(afl::charset::g_codepageLatin1)), fs, 0);

    // Test saveConfiguration
    Ref<Root> root = game::test::makeRoot(game::HostVersion());
    root->userConfiguration().setOption("dings", "bums", game::config::ConfigurationOption::Game);

    Log log;
    NullTranslator tx;
    bool flag = false;
    testee.saveConfiguration(*root, log, tx, game::makeConfirmationTask(true, game::makeResultTask(flag)))
        ->call();
    a.check("saveConfiguration success", flag);

    // Verify
    Ref<afl::io::Stream> config = root->gameDirectory().openFile("pcc2.ini", FileSystem::OpenRead);
    String_t content = afl::string::fromBytes(config->createVirtualMapping()->get());
    a.checkDifferent("content", content.find("dings = bums"), String_t::npos);
}

/** Test loadExpressionLists. */
AFL_TEST("game.v3.BaseTurnLoader:loadExpressionLists", a)
{
    // File system
    InternalFileSystem fs;
    populateFileSystem(fs);

    // Environment
    InternalEnvironment env;
    env.setSettingsDirectoryName("/profile");
    fs.openFile("/profile/lru.ini", FileSystem::Create)
        ->fullWrite(afl::string::toBytes("[planetlabels]\nfoo  foo\n"));

    // Test object
    util::ProfileDirectory profile(env, fs);
    Testee testee(fs.openDirectory("/spec"), std::auto_ptr<Charset>(new CodepageCharset(afl::charset::g_codepageLatin1)), fs, &profile);

    // Load
    Ref<Game> g = *new Game();
    Log log;
    NullTranslator tx;
    testee.loadExpressionLists(*g, log, tx);

    // Verify
    a.checkEqual("loaded expression",
                 g->expressionLists().get(game::config::ExpressionLists::PlanetLabels,
                                          game::config::ExpressionLists::Recent)
                 ->get(0)
                 ->value, "foo");
}

/** Test saveExpressionLists. */
AFL_TEST("game.v3.BaseTurnLoader:saveExpressionLists", a)
{
    // File system
    InternalFileSystem fs;
    populateFileSystem(fs);

    // Environment
    InternalEnvironment env;
    env.setSettingsDirectoryName("/profile");

    // Test object
    util::ProfileDirectory profile(env, fs);
    Testee testee(fs.openDirectory("/spec"), std::auto_ptr<Charset>(new CodepageCharset(afl::charset::g_codepageLatin1)), fs, &profile);

    Ref<Game> g = *new Game();
    g->expressionLists().get(game::config::ExpressionLists::PlanetLabels,
                             game::config::ExpressionLists::Recent)
        ->pushBackNew(new util::ExpressionList::Item("baz", "", "baz"));
    Log log;
    NullTranslator tx;
    testee.saveExpressionLists(*g, log, tx);

    // Verify
    Ref<afl::io::Stream> config = fs.openFile("/profile/lru.ini", FileSystem::OpenRead);
    String_t content = afl::string::fromBytes(config->createVirtualMapping()->get());
    a.checkDifferent("content", content.find("baz  baz"), String_t::npos);
}
