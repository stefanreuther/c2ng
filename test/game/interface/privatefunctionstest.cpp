/**
  *  \file test/game/interface/privatefunctionstest.cpp
  *  \brief Test for game::interface::PrivateFunctions
  */

#include "game/interface/privatefunctions.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/test/testrunner.hpp"
#include "game/browser/directoryhandler.hpp"
#include "game/game.hpp"
#include "game/spec/shiplist.hpp"
#include "game/specificationloader.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/stringverifier.hpp"
#include "game/turn.hpp"
#include "interpreter/process.hpp"
#include "util/requestreceiver.hpp"
#include "util/simplerequestdispatcher.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using game::Game;
using game::PlayerSet_t;
using game::Root;
using game::Session;
using game::StatusTask_t;
using game::Task_t;
using game::Turn;
using game::browser::Account;
using game::browser::Folder;
using game::browser::Handler;
using game::browser::LoadGameRootTask_t;
using game::interface::PrivateFunctions;
using game::spec::ShipList;
using interpreter::BCORef_t;
using interpreter::BytecodeObject;
using interpreter::Process;

namespace {
    /*
     *  Test Environment
     */
    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::InternalFileSystem fs;
        Session session;
        Environment()
            : tx(), fs(), session(tx, fs)
            { }
    };

    // Run process; process must run to end.
    // If dispatcher is given, execute it.
    void run(afl::test::Assert a, Environment& env, BCORef_t bco, Process::State expectedState, util::SimpleRequestDispatcher* pDisp)
    {
        Process& proc = env.session.processList().create(env.session.world(), "t");
        proc.pushFrame(bco, false);

        uint32_t pgid = env.session.processList().allocateProcessGroup();
        env.session.processList().resumeProcess(proc, pgid);
        env.session.processList().startProcessGroup(pgid);
        env.session.runScripts();

        if (pDisp != 0) {
            while (pDisp->wait(100)) {
                // nix
            }
        }

        a.checkEqual("process state", proc.getState(), expectedState);
    }

    /*
     *  SpecificationLoader instance for testing
     */
    class TestSpecLoader : public game::SpecificationLoader {
     public:
        TestSpecLoader(bool result)
            : m_result(result), m_beenHere(false)
            { }
        virtual std::auto_ptr<Task_t> loadShipList(game::spec::ShipList& /*list*/, Root& /*root*/, std::auto_ptr<StatusTask_t> then)
            { m_beenHere = true; return game::makeConfirmationTask(m_result, then); }
        virtual Ref<afl::io::Stream> openSpecificationFile(const String_t& fileName)
            { throw afl::except::FileProblemException(fileName, "not found"); }
        bool beenHere()
            { return m_beenHere; }
     private:
        bool m_result;
        bool m_beenHere;
    };

    Ptr<Root> makeRootWithSpecLoader(Ref<game::SpecificationLoader> spec)
    {
        return new Root(afl::io::InternalDirectory::create("dir"),
                        spec,
                        game::HostVersion(),
                        std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Registered, 9)),
                        std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                        std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                        Root::Actions_t());
    }

    /*
     *  TurnLoader instance for testing
     */
    class TestTurnLoader : public game::TurnLoader {
     public:
        TestTurnLoader(bool result)
            : m_result(result), m_playerLog(0)
            { }
        virtual PlayerStatusSet_t getPlayerStatus(int /*player*/, String_t& /*extra*/, afl::string::Translator& /*tx*/) const
            { return PlayerStatusSet_t(); }
        virtual std::auto_ptr<Task_t> loadCurrentTurn(Game& /*game*/, int player, Root& /*root*/, Session& /*session*/, std::auto_ptr<StatusTask_t> then)
            { m_playerLog = 1000*m_playerLog + player; return game::makeConfirmationTask(m_result, then); }
        virtual std::auto_ptr<Task_t> saveCurrentTurn(const Game& /*game*/, PlayerSet_t /*players*/, SaveOptions_t /*opts*/, const Root& /*root*/, Session& /*session*/, std::auto_ptr<StatusTask_t> /*then*/)
            { throw std::runtime_error("unexpected saveCurrentTurn"); }
        virtual void getHistoryStatus(int /*player*/, int /*turn*/, afl::base::Memory<HistoryStatus> /*status*/, const Root& /*root*/)
            { throw std::runtime_error("unexpected getHistoryStatus"); }
        virtual std::auto_ptr<Task_t> loadHistoryTurn(Turn& /*turn*/, Game& /*game*/, int /*player*/, int /*turnNumber*/, Root& /*root*/, Session& /*session*/, std::auto_ptr<StatusTask_t> /*then*/)
            { throw std::runtime_error("unexpected loadHistoryTurn"); }
        virtual std::auto_ptr<Task_t> saveConfiguration(const Root& /*root*/, afl::sys::LogListener& /*log*/, afl::string::Translator& /*tx*/, std::auto_ptr<Task_t> /*then*/)
            { throw std::runtime_error("unexpected saveConfiguration"); }
        virtual String_t getProperty(Property /*p*/)
            { return ""; }
        int32_t getPlayerLog() const
            { return m_playerLog; }
     private:
        bool m_result;
        int32_t m_playerLog;
    };
}

/* Test addTakeRoot.
   This primarily tests the task juggling. */
AFL_TEST("game.interface.PrivateFunctions:addTakeRoot", a)
{
    // A game directory for a game of type 'test'
    Environment env;
    env.fs.createDirectory("/gamedir");
    env.fs.openFile("/gamedir/pcc2.ini", afl::io::FileSystem::Create)
        ->fullWrite(afl::string::toBytes("Game.Type = test\n"));

    // Browser
    afl::sys::InternalEnvironment sysEnv;
    util::ProfileDirectory profile(sysEnv, env.fs);
    game::browser::Session browserSession(env.fs, env.tx, env.session.log(), profile);

    // Open browser on root directory, and focus on "/gamedir".
    // The easiest way to do so is to re-use DirectoryHandler.
    browserSession.browser().addNewHandler(new game::browser::DirectoryHandler(browserSession.browser(), afl::io::InternalDirectory::create("spec"), profile));
    browserSession.browser().openFolder("/");
    browserSession.browser().loadContent(std::auto_ptr<Task_t>(Task_t::makeNull()))
        ->call();
    browserSession.browser().selectChild(0);

    // Add a Handler that recognizes 'test' games, and produces a root.
    class TestHandler : public Handler {
     public:
        virtual bool handleFolderName(String_t /*name*/, afl::container::PtrVector<Folder>& /*result*/)
            { return false; }
        virtual Folder* createAccountFolder(const afl::base::Ref<Account>& /*acc*/)
            { return 0; }
        virtual std::auto_ptr<Task_t> loadGameRootMaybe(afl::base::Ref<afl::io::Directory> /*dir*/, const game::config::UserConfiguration& config, std::auto_ptr<LoadGameRootTask_t>& then)
            {
                if (config[game::config::UserConfiguration::Game_Type]() == "test") {
                    class Task : public Task_t {
                     public:
                        Task(std::auto_ptr<LoadGameRootTask_t>& then)
                            : m_then(then)
                            { }
                        virtual void call()
                            { m_then->call(makeRootWithSpecLoader(*new TestSpecLoader(true))); }
                     private:
                        std::auto_ptr<LoadGameRootTask_t> m_then;
                    };
                    return std::auto_ptr<Task_t>(new Task(then));
                } else {
                    return std::auto_ptr<Task_t>();
                }
            }
    };
    browserSession.browser().addNewHandler(new TestHandler());

    // Make it possible to send requests to browser and game session.
    util::SimpleRequestDispatcher dispatcher;
    util::RequestReceiver<game::browser::Session> browserReceiver(dispatcher, browserSession);
    util::RequestReceiver<game::Session> gameReceiver(dispatcher, env.session);

    // Do it
    BCORef_t bco = BytecodeObject::create(true);
    PrivateFunctions::addTakeRoot(env.session, *bco, gameReceiver.getSender(), browserReceiver.getSender());
    run(a, env, bco, Process::Ended, &dispatcher);

    // On success, the session has a valid root.
    a.checkNonNull("getRoot", env.session.getRoot().get());
}

/* Test addMakeGame */
AFL_TEST("game.interface.PrivateFunctions:addMakeGame", a)
{
    Environment env;
    BCORef_t bco = BytecodeObject::create(true);
    PrivateFunctions::addMakeGame(env.session, *bco);
    run(a, env, bco, Process::Ended, 0);
    a.checkNonNull("getGame", env.session.getGame().get());
}

/* Test addMakeShipList */
AFL_TEST("game.interface.PrivateFunctions:addMakeShipList", a)
{
    Environment env;
    BCORef_t bco = BytecodeObject::create(true);
    PrivateFunctions::addMakeShipList(env.session, *bco);
    run(a, env, bco, Process::Ended, 0);
    a.checkNonNull("getShipList", env.session.getShipList().get());
}

/* Test addLoadShipList, success case */
AFL_TEST("game.interface.PrivateFunctions:addLoadShipList", a)
{
    Environment env;
    BCORef_t bco = BytecodeObject::create(true);
    PrivateFunctions::addLoadShipList(env.session, *bco);

    // Precondition for addLoadShipList: we must have a ShipList object and a Root with a SpecificationLoader
    Ref<TestSpecLoader> spec(*new TestSpecLoader(true));
    env.session.setRoot(makeRootWithSpecLoader(spec));
    env.session.setShipList(new game::spec::ShipList());

    run(a, env, bco, Process::Ended, 0);
    a.check("been here", spec->beenHere());
}

/* Test addLoadShipList, error case */
AFL_TEST("game.interface.PrivateFunctions:addLoadShipList:error", a)
{
    Environment env;
    BCORef_t bco = BytecodeObject::create(true);
    PrivateFunctions::addLoadShipList(env.session, *bco);

    // Precondition for addLoadShipList: we must have a ShipList object and a Root with a SpecificationLoader
    Ref<TestSpecLoader> spec(*new TestSpecLoader(false));
    env.session.setRoot(makeRootWithSpecLoader(spec));
    env.session.setShipList(new game::spec::ShipList());

    run(a, env, bco, Process::Failed, 0);
    a.check("been here", spec->beenHere());
}

/* Test addLoadCurrentTurn, success case */
AFL_TEST("game.interface.PrivateFunctions:addLoadCurrentTurn", a)
{
    Environment env;
    BCORef_t bco = BytecodeObject::create(true);
    PrivateFunctions::addLoadCurrentTurn(env.session, *bco, 12);

    // Precondition for addLoadShipList: we must have Game and Root objects. Use the TestSpecLoader for simplicity.
    Ref<TestSpecLoader> spec(*new TestSpecLoader(false));
    Ref<TestTurnLoader> turn(*new TestTurnLoader(true));
    env.session.setRoot(makeRootWithSpecLoader(spec));
    env.session.getRoot()->setTurnLoader(turn.asPtr());
    env.session.setGame(new Game());

    run(a, env, bco, Process::Ended, 0);
    a.checkEqual("turn player log", turn->getPlayerLog(), 12);
}

/* Test addLoadCurrentTurn, error case */
AFL_TEST("game.interface.PrivateFunctions:addLoadCurrentTurn:error", a)
{
    Environment env;
    BCORef_t bco = BytecodeObject::create(true);
    PrivateFunctions::addLoadCurrentTurn(env.session, *bco, 12);

    // Precondition for addLoadShipList: we must have Game and Root objects. Use the TestSpecLoader for simplicity.
    Ref<TestSpecLoader> spec(*new TestSpecLoader(false));
    Ref<TestTurnLoader> turn(*new TestTurnLoader(false));
    env.session.setRoot(makeRootWithSpecLoader(spec));
    env.session.getRoot()->setTurnLoader(turn.asPtr());
    env.session.setGame(new Game());

    run(a, env, bco, Process::Failed, 0);
    a.checkEqual("turn player log", turn->getPlayerLog(), 12);
}

/* Test addPostprocessCurrentTurn.
   Just farming coverage here. */
AFL_TEST("game.interface.PrivateFunctions:addPostprocessCurrentTurn", a)
{
    Environment env;
    BCORef_t bco = BytecodeObject::create(true);
    PrivateFunctions::addPostprocessCurrentTurn(env.session, *bco, 5);

    env.session.setRoot(makeRootWithSpecLoader(*new TestSpecLoader(false)));
    env.session.setGame(new Game());
    env.session.setShipList(new ShipList());

    run(a, env, bco, Process::Ended, 0);
    a.checkEqual("viewpoint", env.session.getGame()->getViewpointPlayer(), 5);
}
