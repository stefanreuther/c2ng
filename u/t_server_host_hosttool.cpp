/**
  *  \file u/t_server_host_hosttool.cpp
  *  \brief Test for server::host::HostTool
  */

#include "server/host/hosttool.hpp"

#include "t_server_host.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

using server::host::HostTool;

namespace {
    class TestHarness {
     public:
        TestHarness()
            : m_hostfile(),
              m_db(), m_null(), m_mail(m_null), m_runner(), m_fs(),
              m_root(m_db, m_hostfile, m_null, m_mail, m_runner, m_fs, server::host::Configuration())
            { }

        server::host::Root& root()
            { return m_root; }

        afl::net::CommandHandler& db()
            { return m_db; }

        afl::net::CommandHandler& hostFile()
            { return m_hostfile; }

     private:
        server::file::InternalFileServer m_hostfile;

        afl::net::redis::InternalDatabase m_db;
        afl::net::NullCommandHandler m_null;
        server::interface::MailQueueClient m_mail;
        util::ProcessRunner m_runner;
        afl::io::NullFileSystem m_fs;
        server::host::Root m_root;
    };

    struct Compare {
        bool operator()(const HostTool::Info& a, const HostTool::Info& b) const
            { return a.id < b.id; }
    };
}

/** Test basic operation: add, set, get. */
void
TestServerHostHostTool::testBasic()
{
    TestHarness h;
    server::host::Session session;
    HostTool testee(session, h.root(), h.root().toolRoot());

    // Create a tool that does not need a file
    TS_ASSERT_THROWS_NOTHING(testee.add("tool-id", "", "", "toolkind"));
    TS_ASSERT_THROWS_NOTHING(testee.set("tool-id", "description", "Lengthy text..."));
    TS_ASSERT_EQUALS(testee.get("tool-id", "description"), "Lengthy text...");

    // Try to create a tool that needs a file.
    // This fails because the file does not exist.
    TS_ASSERT_THROWS(testee.add("tool-file", "dir", "file", "tool-kind"), std::exception);

    // OK, create the file and try again.
    server::interface::FileBaseClient(h.hostFile()).createDirectory("dir");
    server::interface::FileBaseClient(h.hostFile()).putFile("dir/file", "content");
    TS_ASSERT_THROWS_NOTHING(testee.add("tool-file", "dir", "file", "toolkind"));
}

/** Test list operations: add, getAll, remove, setDefault. */
void
TestServerHostHostTool::testList()
{
    TestHarness h;
    server::host::Session session;
    HostTool testee(session, h.root(), h.root().toolRoot());

    // Create some tools
    testee.add("a", "", "", "ak");
    testee.add("b", "", "", "bk");
    testee.add("c", "", "", "ck");

    // Fetch
    {
        std::vector<HostTool::Info> result;
        testee.getAll(result);
        TS_ASSERT_EQUALS(result.size(), 3U);

        std::sort(result.begin(), result.end(), Compare());
        TS_ASSERT_EQUALS(result[0].id,   "a");
        TS_ASSERT_EQUALS(result[0].kind, "ak");
        TS_ASSERT_EQUALS(result[1].id,   "b");
        TS_ASSERT_EQUALS(result[1].kind, "bk");
        TS_ASSERT_EQUALS(result[2].id,   "c");
        TS_ASSERT_EQUALS(result[2].kind, "ck");
        TS_ASSERT(result[0].isDefault);
        TS_ASSERT(!result[1].isDefault);
        TS_ASSERT(!result[2].isDefault);
    }

    // Make one default
    testee.setDefault("c");
    {
        std::vector<HostTool::Info> result;
        testee.getAll(result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        std::sort(result.begin(), result.end(), Compare());
        TS_ASSERT(!result[0].isDefault);
        TS_ASSERT(!result[1].isDefault);
        TS_ASSERT(result[2].isDefault);
    }

    // Remove c
    testee.remove("c");
    {
        std::vector<HostTool::Info> result;
        testee.getAll(result);
        TS_ASSERT_EQUALS(result.size(), 2U);
        std::sort(result.begin(), result.end(), Compare());
        TS_ASSERT_EQUALS(result[0].id, "a");
        TS_ASSERT_EQUALS(result[1].id, "b");
        TS_ASSERT(result[0].isDefault || result[1].isDefault);
    }
}

/** Test copy. */
void
TestServerHostHostTool::testCopy()
{
    TestHarness h;
    server::host::Session session;
    HostTool testee(session, h.root(), h.root().toolRoot());

    // Create a tool
    TS_ASSERT_THROWS_NOTHING(testee.add("a", "", "", "kk"));
    TS_ASSERT_THROWS_NOTHING(testee.set("a", "description", "Lengthy text..."));
    TS_ASSERT_THROWS_NOTHING(testee.set("a", "docurl", "http://"));

    // Copy
    TS_ASSERT_THROWS_NOTHING(testee.copy("a", "x"));

    // Verify
    {
        std::vector<HostTool::Info> result;
        testee.getAll(result);
        TS_ASSERT_EQUALS(result.size(), 2U);
        std::sort(result.begin(), result.end(), Compare());
        TS_ASSERT_EQUALS(result[0].id, "a");
        TS_ASSERT_EQUALS(result[1].id, "x");
        TS_ASSERT(result[0].isDefault || result[1].isDefault);
    }
    TS_ASSERT_EQUALS(testee.get("x", "docurl"), "http://");
}

/** Test various error cases. */
void
TestServerHostHostTool::testErrors()
{
    TestHarness h;
    server::host::Session session;
    HostTool testee(session, h.root(), h.root().toolRoot());

    TS_ASSERT_THROWS_NOTHING(testee.add("x", "", "", "k"));

    // Bad Id
    TS_ASSERT_THROWS(testee.add("", "", "", "k"), std::exception);
    TS_ASSERT_THROWS(testee.add("a b", "", "", "k"), std::exception);
    TS_ASSERT_THROWS(testee.add("a\xC3\xB6", "", "", "k"), std::exception);
    TS_ASSERT_THROWS(testee.set("", "k", "v"), std::exception);
    TS_ASSERT_THROWS(testee.copy("x", ""), std::exception);

    // Bad Kind
    TS_ASSERT_THROWS(testee.add("a", "", "", ""), std::exception);
    TS_ASSERT_THROWS(testee.add("a", "", "", "a b"), std::exception);
    TS_ASSERT_THROWS(testee.add("a", "", "", "a-b"), std::exception);

    // Nonexistant
    TS_ASSERT_THROWS(testee.copy("a", "b"), std::exception);
    TS_ASSERT_THROWS(testee.setDefault("a"), std::exception);
    TS_ASSERT_THROWS(testee.getDifficulty("a"), std::exception);
    TS_ASSERT_THROWS(testee.clearDifficulty("a"), std::exception);
    TS_ASSERT_THROWS(testee.setDifficulty("a", 99, true), std::exception);

    // Missing tool
    TS_ASSERT_THROWS(testee.add("a", "b", "c", "d"), std::exception);
}

/** Test difficulty access commands. */
void
TestServerHostHostTool::testDifficulty()
{
    TestHarness h;
    server::host::Session session;
    HostTool testee(session, h.root(), h.root().toolRoot());

    // Add a tool
    testee.add("t", "", "", "k");
    TS_ASSERT_EQUALS(testee.getDifficulty("t"), 0);

    // Set difficulty
    testee.setDifficulty("t", 33, true);
    TS_ASSERT_EQUALS(testee.getDifficulty("t"), 33);

    // Remove difficulty
    testee.clearDifficulty("t");
    TS_ASSERT_EQUALS(testee.getDifficulty("t"), 0);
}

/** Test difficulty computation. */
void
TestServerHostHostTool::testComputedDifficulty()
{
    TestHarness h;
    server::host::Session session;
    HostTool testee(session, h.root(), h.root().toolRoot());

    // Upload a config file for an ultra-rich game
    server::interface::FileBaseClient(h.hostFile()).createDirectory("dir");
    server::interface::FileBaseClient(h.hostFile()).putFile("dir/amaster.src",
                                                            "%amaster\n"
                                                            "planetcorerangesalternate=10000,20000\n"
                                                            "planetcorerangesusual=10000,20000\n"
                                                            "planetcoreusualfrequency=50\n"
                                                            "planetsurfaceranges=5000,10000\n");
    // Add as tool
    testee.add("easy", "dir", "", "config");

    // Compute difficulty
    int32_t n = testee.setDifficulty("easy", afl::base::Nothing, true);
    TS_ASSERT_EQUALS(n, 28);
    TS_ASSERT_EQUALS(testee.getDifficulty("easy"), 28);

    // Change the file to make it harder
    server::interface::FileBaseClient(h.hostFile()).putFile("dir/amaster.src",
                                                            "%amaster\n"
                                                            "planetcorerangesalternate=100,200\n"
                                                            "planetcorerangesusual=100,200\n"
                                                            "planetcoreusualfrequency=50\n"
                                                            "planetsurfaceranges=50,100\n");
    n = testee.setDifficulty("easy", afl::base::Nothing, true);
    TS_ASSERT_EQUALS(n, 126);
}

