/**
  *  \file test/game/proxy/maintenanceproxytest.cpp
  *  \brief Test for game::proxy::MaintenanceProxy
  */

#include "game/proxy/maintenanceproxy.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/test/testrunner.hpp"
#include "game/actions/preconditions.hpp"
#include "game/root.hpp"
#include "game/test/counter.hpp"
#include "game/test/files.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/v3/utils.hpp"

using afl::base::Ref;
using afl::io::InternalDirectory;
using afl::io::FileSystem;
using game::Timestamp;
using game::proxy::MaintenanceProxy;

namespace {
    /*
     *  Adaptor used for testing
     */

    class Adaptor : public game::proxy::MaintenanceAdaptor {
     public:
        Adaptor(game::Session& session, const Ref<afl::io::Directory>& dir)
            : m_session(session), m_directory(dir)
            { }
        virtual afl::io::Directory& targetDirectory()
            { return *m_directory; }
        virtual afl::string::Translator& translator()
            { return m_session.translator(); }
        virtual afl::charset::Charset& charset()
            { return game::actions::mustHaveRoot(m_session).charset(); }
        virtual const game::PlayerList& playerList()
            { return game::actions::mustHaveRoot(m_session).playerList(); }
        virtual afl::io::FileSystem& fileSystem()
            { return m_session.world().fileSystem(); }
        virtual game::config::UserConfiguration& userConfiguration()
            { return game::actions::mustHaveRoot(m_session).userConfiguration(); }
     private:
        game::Session& m_session;
        Ref<afl::io::Directory> m_directory;
    };

    class AdaptorFromSession : public afl::base::Closure<game::proxy::MaintenanceAdaptor*(game::Session&)> {
     public:
        AdaptorFromSession(const Ref<afl::io::Directory>& dir)
            : m_directory(dir)
            { }
        virtual game::proxy::MaintenanceAdaptor* call(game::Session& session)
            { return new Adaptor(session, m_directory); }
     private:
        Ref<afl::io::Directory> m_directory;
    };


    /*
     *  Testing environment
     *
     *  Provides the multi-threading setup and a game directory for the MaintenanceProxy.
     *  By default, the setup has no Root (which would fail all operations); use addRoot().
     *  By default, event counters are not connected; use addCounters().
     */
    struct Environment {
        game::test::SessionThread sessionThread;
        game::test::WaitIndicator waitIndicator;
        Ref<InternalDirectory> dir;
        MaintenanceProxy testee;
        game::test::Counter messageCounter;
        game::test::Counter completionCounter;

        Environment()
            : sessionThread(), waitIndicator(), dir(InternalDirectory::create("dir")),
              testee(sessionThread.gameSender().makeTemporary(new AdaptorFromSession(dir)), waitIndicator),
              messageCounter(), completionCounter()
            { }

        game::Session& session()
            { return sessionThread.session(); }
    };

    // Data for an empty file
    const uint8_t EMPTY[] = {0,0};

    // Add a Root
    void addRoot(Environment& env)
    {
        Ref<game::Root> root = game::test::makeRoot(game::HostVersion());

        // Default race names
        Ref<InternalDirectory> dir = InternalDirectory::create("spec");
        dir->openFile("race.nm", FileSystem::Create)->fullWrite(game::test::getDefaultRaceNames());
        game::v3::loadRaceNames(root->playerList(), *dir, root->charset());

        env.session().setRoot(root.asPtr());
    }

    // Add a file in the game directory to the test environment
    void addFile(Environment& env, const String_t& name, afl::base::ConstBytes_t data)
    {
        env.dir->openFile(name, FileSystem::Create)->fullWrite(data);
    }

    // Check presence of a file
    bool hasFile(Environment& env, const String_t& name)
    {
        return env.dir->openFileNT(name, FileSystem::OpenRead).get() != 0;
    }

    // Get size of a file
    int32_t getFileSize(Environment& env, const String_t& name)
    {
        return static_cast<int32_t>(env.dir->openFile(name, FileSystem::OpenRead)->getSize());
    }

    // Connect the counters to events from MaintenanceProxy
    void addCounters(Environment& env)
    {
        env.testee.sig_message.add(&env.messageCounter, &game::test::Counter::increment);
        env.testee.sig_actionComplete.add(&env.completionCounter, &game::test::Counter::increment);
    }

    // Wait for completion of operation (=sig_actionComplete)
    void waitForCompletion(Environment& env)
    {
        while (env.completionCounter.get() == 0) {
            env.waitIndicator.processQueue();
            env.sessionThread.sync();
        }
    }
}

/** Test behaviour with empty/disfunctional session.
    Verifies that the "prepare" function correctly reports valid=false. */
AFL_TEST("game.proxy.MaintenanceProxy:empty", a)
{
    Environment env;
    a.checkEqual("01. prepareUnpack",   env.testee.prepareUnpack(env.waitIndicator).valid, false);
    a.checkEqual("02. prepareMaketurn", env.testee.prepareMaketurn(env.waitIndicator).valid, false);
    a.checkEqual("03. prepareSweep",    env.testee.prepareSweep(env.waitIndicator).valid, false);
}

/** Test unpack, base case. */
AFL_TEST("game.proxy.MaintenanceProxy:unpack", a)
{
    Environment env;
    addRoot(env);
    addFile(env, "player2.rst", game::test::makeEmptyResult(2, 70, Timestamp(2003, 12, 10, 12, 0, 0)));
    addFile(env, "player4.rst", game::test::makeEmptyResult(4, 70, Timestamp(2003, 12, 10, 12, 0, 0)));
    addFile(env, "player7.rst", game::test::makeEmptyResult(7, 69, Timestamp(2003, 11, 10, 12, 0, 0)));
    env.session().getRoot()->userConfiguration()[game::config::UserConfiguration::Unpack_AttachmentTimestamp].set("1-2-3");

    // Prepare
    MaintenanceProxy::UnpackStatus st = env.testee.prepareUnpack(env.waitIndicator);
    a.checkEqual("01. valid",            st.valid, true);
    a.checkEqual("02. allPlayers",       st.allPlayers.toInteger(), 0xFFEU);
    a.checkEqual("03. availablePlayers", st.availablePlayers.toInteger(), 0x094U);
    a.checkEqual("04. selectedPlayers",  st.selectedPlayers.toInteger(), 0x000U);
    a.checkEqual("05. turnFilePlayers",  st.turnFilePlayers.toInteger(), 0x000U);
    a.checkEqual("06. playerNames",      st.playerNames.get(1), "The Feds");

    // Unpack
    addCounters(env);
    env.testee.startUnpack(game::PlayerSet_t() + 4 + 7, false);
    waitForCompletion(env);
    a.checkEqual("11. completionCounter", env.completionCounter.get(), 1);
    a.check("12. messageCounter", env.messageCounter.get() >= 1);
    a.check("13. gen2", !hasFile(env, "gen2.dat"));
    a.check("14. gen4", hasFile(env, "gen4.dat"));
    a.check("15. gen7", hasFile(env, "gen7.dat"));

    // Default is Winplan format, so we should have a Winplan outbox
    a.check("21. mess357", hasFile(env, "mess357.dat"));

    // Verify that attachment timestamp has been reset
    a.checkEqual("31. AttachmentTimestamp", env.session().getRoot()->userConfiguration()[game::config::UserConfiguration::Unpack_AttachmentTimestamp](), "");
}

/** Test unpack, with turn file. */
AFL_TEST("game.proxy.MaintenanceProxy:unpack:with-turn", a)
{
    Environment env;
    addRoot(env);
    addFile(env, "player2.rst", game::test::makeEmptyResult(2, 70, Timestamp(2003, 12, 10, 12, 0, 0)));
    addFile(env, "player4.rst", game::test::makeEmptyResult(4, 70, Timestamp(2003, 12, 10, 12, 0, 0)));

    addFile(env, "player2.trn", game::test::makeSimpleTurn(2, Timestamp(2002, 12, 10, 12, 0, 0)));  // Does not match
    addFile(env, "player4.trn", game::test::makeSimpleTurn(4, Timestamp(2003, 12, 10, 12, 0, 0)));  // Matches

    // Prepare
    MaintenanceProxy::UnpackStatus st = env.testee.prepareUnpack(env.waitIndicator);
    a.checkEqual("01. valid", st.valid, true);
    a.checkEqual("02. allPlayers", st.allPlayers.toInteger(), 0xFFEU);
    a.checkEqual("03. availablePlayers", st.availablePlayers.toInteger(), 0x014U);
    a.checkEqual("04. selectedPlayers", st.selectedPlayers.toInteger(), 0x000U);
    a.checkEqual("05. turnFilePlayers", st.turnFilePlayers.toInteger(), 0x010U);

    // Unpack
    addCounters(env);
    env.testee.startUnpack(game::PlayerSet_t() + 2 + 4, true);
    waitForCompletion(env);
    a.checkEqual("11. completionCounter", env.completionCounter.get(), 1);
    a.check("12. messageCounter", env.messageCounter.get() >= 1);
    a.check("13. gen2", hasFile(env, "gen2.dat"));
    a.check("14. gen4", hasFile(env, "gen4.dat"));
    a.checkEqual("15. mess352", getFileSize(env, "mess352.dat"), 2);
    a.checkEqual("16. mess354", getFileSize(env, "mess354.dat"), 635);
}

AFL_TEST("game.proxy.MaintenanceProxy:unpack:existing", a)
{
    Environment env;
    addRoot(env);
    addFile(env, "player2.rst", game::test::makeEmptyResult(2, 70, Timestamp(2003, 12, 10, 12, 0, 0)));
    addFile(env, "player4.rst", game::test::makeEmptyResult(4, 70, Timestamp(2003, 12, 10, 12, 0, 0)));
    addFile(env, "gen2.dat", game::test::makeGenFile(2, 70, Timestamp(2003, 12, 10, 12, 0, 0)));

    // Prepare
    MaintenanceProxy::UnpackStatus st = env.testee.prepareUnpack(env.waitIndicator);
    a.checkEqual("01. valid", st.valid, true);
    a.checkEqual("02. allPlayers", st.allPlayers.toInteger(), 0xFFEU);
    a.checkEqual("03. availablePlayers", st.availablePlayers.toInteger(), 0x014U);
    a.checkEqual("04. selectedPlayers", st.selectedPlayers.toInteger(), 0x004U);
    a.checkEqual("05. turnFilePlayers", st.turnFilePlayers.toInteger(), 0x000U);
}

/** Test unpack configuration.
    Default is Windows format. Configure to DOS and check that configuration is effective. */
AFL_TEST("game.proxy.MaintenanceProxy:unpack:config", a)
{
    Environment env;
    addRoot(env);
    addFile(env, "player7.rst", game::test::makeEmptyResult(7, 69, Timestamp(2003, 11, 10, 12, 0, 0)));
    env.session().getRoot()->userConfiguration()[game::config::UserConfiguration::Unpack_Format].set("DOS");

    // Prepare
    MaintenanceProxy::UnpackStatus st = env.testee.prepareUnpack(env.waitIndicator);
    a.checkEqual("01. valid", st.valid, true);

    // Unpack
    addCounters(env);
    env.testee.startUnpack(game::PlayerSet_t() + 7, false);
    waitForCompletion(env);
    a.checkEqual("11. completionCounter", env.completionCounter.get(), 1);
    a.check("12. messageCounter", env.messageCounter.get() >= 1);
    a.check("13. gen7", hasFile(env, "gen7.dat"));
    a.check("14. mess7", hasFile(env, "mess7.dat"));
}

/** Test Maketurn. */
AFL_TEST("game.proxy.MaintenanceProxy:maketurn", a)
{
    Environment env;
    addRoot(env);
    addFile(env, "bdata3.dat", EMPTY);
    addFile(env, "bdata3.dis", EMPTY);
    addFile(env, "pdata3.dat", EMPTY);
    addFile(env, "pdata3.dis", EMPTY);
    addFile(env, "ship3.dat", EMPTY);
    addFile(env, "ship3.dis", EMPTY);
    addFile(env, "fizz.bin", game::test::getDefaultRegKey());
    addFile(env, "gen3.dat", game::test::makeGenFile(3, 30, Timestamp(2004, 4, 1, 13, 0, 5)));

    // Ad-hoc outbox
    static const uint8_t OUTBOX[] = {
        1,0,                    // Count
        13,0,0,0,               // Position
        2,0,                    // Length
        3,0,                    // From
        12,0,                   // To
        'a','b'                 // Text
    };
    addFile(env, "mess3.dat", OUTBOX);

    // Prepare
    MaintenanceProxy::MaketurnStatus st = env.testee.prepareMaketurn(env.waitIndicator);
    a.checkEqual("01. valid", st.valid, true);
    a.checkEqual("02. availablePlayers", st.availablePlayers.toInteger(), 0x008U);

    // Maketurn
    addCounters(env);
    env.testee.startMaketurn(game::PlayerSet_t() + 3);
    waitForCompletion(env);
    a.checkEqual("11. completionCounter", env.completionCounter.get(), 1);
    a.check("12. messageCounter", env.messageCounter.get() >= 1);
    a.check("13. player3.trn", hasFile(env, "player3.trn"));
}

/** Test sweep, base case. */
AFL_TEST("game.proxy.MaintenanceProxy:sweep", a)
{
    Environment env;
    addRoot(env);
    addFile(env, "gen3.dat", game::test::makeGenFile(3, 30, Timestamp(2004, 4, 1, 13, 0, 5)));
    addFile(env, "chart3.cc", EMPTY);
    addFile(env, "pdata5.dis", EMPTY);
    addFile(env, "pdata7.dis", EMPTY);

    // Prepare
    MaintenanceProxy::SweepStatus st = env.testee.prepareSweep(env.waitIndicator);
    a.checkEqual("01. valid", st.valid, true);
    a.checkEqual("02. allPlayers", st.allPlayers.toInteger(), 0xFFEU);
    a.checkEqual("03. selectedPlayers", st.selectedPlayers.toInteger(), 0x000U);

    // Sweep
    addCounters(env);
    env.testee.startSweep(game::PlayerSet_t() + 3 + 5, false);
    waitForCompletion(env);
    a.checkEqual("11. gen3", hasFile(env, "gen3.dat"), false);
    a.checkEqual("12. chart3", hasFile(env, "chart3.cc"), true);     // preserved due to eraseDatabase=false
    a.checkEqual("13. pdata5", hasFile(env, "pdata5.dis"), false);
    a.checkEqual("14. pdata7", hasFile(env, "pdata7.dis"), true);    // not selected
}

/** Test sweep, configure database erasure. */
AFL_TEST("game.proxy.MaintenanceProxy:sweep:config", a)
{
    Environment env;
    addRoot(env);
    addFile(env, "gen3.dat", game::test::makeGenFile(3, 30, Timestamp(2004, 4, 1, 13, 0, 5)));
    addFile(env, "chart3.cc", EMPTY);

    // Prepare
    MaintenanceProxy::SweepStatus st = env.testee.prepareSweep(env.waitIndicator);
    a.checkEqual("01. valid", st.valid, true);
    a.checkEqual("02. allPlayers", st.allPlayers.toInteger(), 0xFFEU);
    a.checkEqual("03. selectedPlayers", st.selectedPlayers.toInteger(), 0x000U);

    // Sweep
    addCounters(env);
    env.testee.startSweep(game::PlayerSet_t() + 3 + 5, true);
    waitForCompletion(env);
    a.checkEqual("11. gen3", hasFile(env, "gen3.dat"), false);
    a.checkEqual("12. chart3", hasFile(env, "chart3.cc"), false);
}

/** Test sweep, when conflicts are present.
    Conflicting races are auto-selected. */
AFL_TEST("game.proxy.MaintenanceProxy:sweep:conflict", a)
{
    Environment env;
    addRoot(env);
    addFile(env, "gen1.dat", game::test::makeGenFile(1, 30, Timestamp(2004, 4, 1, 13, 0, 5)));
    addFile(env, "gen2.dat", game::test::makeGenFile(2, 30, Timestamp(2004, 4, 1, 13, 0, 5)));
    addFile(env, "gen3.dat", game::test::makeGenFile(3, 29, Timestamp(2004, 3, 1, 13, 0, 5)));
    addFile(env, "gen4.dat", game::test::makeGenFile(4, 30, Timestamp(2004, 4, 1, 13, 0, 5)));

    // Prepare
    MaintenanceProxy::SweepStatus st = env.testee.prepareSweep(env.waitIndicator);
    a.checkEqual("01. valid", st.valid, true);
    a.checkEqual("02. allPlayers", st.allPlayers.toInteger(), 0xFFEU);
    a.checkEqual("03. selectedPlayers", st.selectedPlayers.toInteger(), 0x008U);   // Player 3 is auto-selected due to conflict
}
