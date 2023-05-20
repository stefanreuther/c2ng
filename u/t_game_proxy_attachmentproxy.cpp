/**
  *  \file u/t_game_proxy_attachmentproxy.cpp
  *  \brief Test for game::proxy::AttachmentProxy
  */

#include "game/proxy/attachmentproxy.hpp"

#include "t_game_proxy.hpp"
#include "afl/io/internaldirectory.hpp"
#include "game/actions/preconditions.hpp"
#include "game/test/counter.hpp"
#include "game/test/files.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/v3/utils.hpp"

using afl::io::FileSystem;
using afl::base::Ref;
using afl::io::InternalDirectory;
using game::proxy::AttachmentProxy;

#define UTILDAT_PREFIX_WITH_TIMESTAMP(TIMESTAMP)                        \
    0x0d, 0x00, 0x59, 0x00,                                             \
    TIMESTAMP,  0x0c, 0x00, 0x01, 0x00, 0x04, 0x00, 0x04, 0xda, 0xb0, 0x10, 0xec, 0x94, 0x3d, 0x36, \
    0x04, 0xad, 0xe9, 0x90, 0x38, 0xd4, 0x8d, 0xb7, 0x11, 0x5e, 0xef, 0x6a, 0x0e, 0x79, 0xe8, 0x84, \
    0xc0, 0xbd, 0x6f, 0x03, 0xe7, 0xbe, 0xed, 0xeb, 0x46, 0x4c, 0x41, 0x4b, 0x30, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6a

#define RST35_TIMESTAMP     0x30, 0x32, 0x2d, 0x30, 0x38, 0x2d, 0x32, 0x30, 0x31, 0x36, 0x31, 0x34, 0x3a, 0x34, 0x38, 0x3a, 0x30, 0x33
#define UTILDAT_PREFIX      UTILDAT_PREFIX_WITH_TIMESTAMP(RST35_TIMESTAMP)

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
     *  Testing environment (same as for MaintenanceProxy)
     */
    struct Environment {
        game::test::SessionThread sessionThread;
        game::test::WaitIndicator waitIndicator;
        Ref<InternalDirectory> dir;
        AttachmentProxy testee;
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

/** Test normal case. */
void
TestGameProxyAttachmentProxy::testIt()
{
    static const uint8_t FILE[] = {
        UTILDAT_PREFIX,
        34,0, 2+13,0, 'a','.','d','a','t',0,0,0,0,0,0,0, 0, 'x','y',
    };

    // Environment
    Environment env;
    addRoot(env);
    addFile(env, "util1.dat", FILE);

    AttachmentProxy::Infos_t result;
    bool flag = true;
    env.testee.loadDirectory(env.waitIndicator, game::PlayerSet_t(1), false, result, flag);
    TS_ASSERT_EQUALS(result.size(), 1U);
    TS_ASSERT(!flag);

    // Unpack
    addCounters(env);
    env.testee.saveFiles();
    waitForCompletion(env);
    TS_ASSERT_EQUALS(env.completionCounter.get(), 1);
    TS_ASSERT_LESS_THAN_EQUALS(1, env.messageCounter.get());
    TS_ASSERT(hasFile(env, "a.dat"));
}

/** Test configuration: disable some files. */
void
TestGameProxyAttachmentProxy::testConfigDisable()
{
    static const uint8_t FILE[] = {
        UTILDAT_PREFIX,
        34,0, 2+13,0, 'a','.','d','a','t',0,0,0,0,0,0,0, 0, 'x','y',
        34,0, 2+13,0, 'b','.','d','a','t',0,0,0,0,0,0,0, 0, 'x','y',
        34,0, 2+13,0, 'c','.','d','a','t',0,0,0,0,0,0,0, 0, 'x','y',
    };

    // Environment
    Environment env;
    addRoot(env);
    addFile(env, "util1.dat", FILE);

    AttachmentProxy::Infos_t result;
    bool flag = true;
    env.testee.loadDirectory(env.waitIndicator, game::PlayerSet_t(1), false, result, flag);
    TS_ASSERT_EQUALS(result.size(), 3U);
    TS_ASSERT(!flag);

    result[0].selected = false;
    env.testee.selectAttachments(result);
    env.testee.selectAttachment("c.dat", false);

    // Unpack
    addCounters(env);
    env.testee.saveFiles();
    waitForCompletion(env);
    TS_ASSERT_EQUALS(env.completionCounter.get(), 1);
    TS_ASSERT_LESS_THAN_EQUALS(1, env.messageCounter.get());
    TS_ASSERT(!hasFile(env, "a.dat"));
    TS_ASSERT(hasFile(env, "b.dat"));
    TS_ASSERT(!hasFile(env, "c.dat"));
}

/** Test repeated operation. */
void
TestGameProxyAttachmentProxy::testRepeat()
{
    static const uint8_t FILE[] = {
        UTILDAT_PREFIX,
        34,0, 2+13,0, 'a','.','d','a','t',0,0,0,0,0,0,0, 0, 'x','y',
        34,0, 2+13,0, 'b','.','d','a','t',0,0,0,0,0,0,0, 0, 'x','y',
        34,0, 2+13,0, 'c','.','d','a','t',0,0,0,0,0,0,0, 0, 'x','y',
    };

    // Environment
    Environment env;
    addRoot(env);
    addFile(env, "util1.dat", FILE);

    {
        AttachmentProxy::Infos_t result;
        bool flag = true;
        env.testee.loadDirectory(env.waitIndicator, game::PlayerSet_t(1), false, result, flag);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT(!flag);
    }

    // Unselect attachments
    env.testee.selectAttachment("a.dat", false);
    env.testee.selectAttachment("b.dat", false);
    env.testee.selectAttachment("c.dat", false);

    // Unpack. This saves that Unpack.AttachmentTimestamp.
    addCounters(env);
    env.testee.saveFiles();
    waitForCompletion(env);

    // Load again without auto-select shows these attachments again
    {
        AttachmentProxy::Infos_t result;
        bool flag = true;
        env.testee.loadDirectory(env.waitIndicator, game::PlayerSet_t(1), false, result, flag);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT(!flag);
    }

    // Load again with auto-select reports empty
    {
        AttachmentProxy::Infos_t result;
        bool flag = true;
        env.testee.loadDirectory(env.waitIndicator, game::PlayerSet_t(1), true, result, flag);
        TS_ASSERT_EQUALS(result.size(), 0U);
        // Value of flag does not matter if result is empty
    }
}

