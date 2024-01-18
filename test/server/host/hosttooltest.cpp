/**
  *  \file test/server/host/hosttooltest.cpp
  *  \brief Test for server::host::HostTool
  */

#include "server/host/hosttool.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("server.host.HostTool:basics", a)
{
    TestHarness h;
    server::host::Session session;
    HostTool testee(session, h.root(), h.root().toolRoot(), HostTool::Tool);

    // Create a tool that does not need a file
    AFL_CHECK_SUCCEEDS(a("01. add"), testee.add("tool-id", "", "", "toolkind"));
    AFL_CHECK_SUCCEEDS(a("02. set"), testee.set("tool-id", "description", "Lengthy text..."));
    a.checkEqual("03. get", testee.get("tool-id", "description"), "Lengthy text...");

    // Try to create a tool that needs a file.
    // This fails because the file does not exist.
    AFL_CHECK_THROWS(a("11. add"), testee.add("tool-file", "dir", "file", "tool-kind"), std::exception);

    // OK, create the file and try again.
    server::interface::FileBaseClient(h.hostFile()).createDirectory("dir");
    server::interface::FileBaseClient(h.hostFile()).putFile("dir/file", "content");
    AFL_CHECK_SUCCEEDS(a("21. add"), testee.add("tool-file", "dir", "file", "toolkind"));
}

/** Test list operations: add, getAll, remove, setDefault. */
AFL_TEST("server.host.HostTool:list", a)
{
    TestHarness h;
    server::host::Session session;
    HostTool testee(session, h.root(), h.root().toolRoot(), HostTool::Tool);

    // Create some tools
    testee.add("a", "", "", "ak");
    testee.add("b", "", "", "bk");
    testee.add("c", "", "", "ck");

    // Fetch
    {
        std::vector<HostTool::Info> result;
        testee.getAll(result);
        a.checkEqual("01. size", result.size(), 3U);

        std::sort(result.begin(), result.end(), Compare());
        a.checkEqual("11. id",    result[0].id,   "a");
        a.checkEqual("12. kind",  result[0].kind, "ak");
        a.checkEqual("13. id",    result[1].id,   "b");
        a.checkEqual("14. kind",  result[1].kind, "bk");
        a.checkEqual("15. id",    result[2].id,   "c");
        a.checkEqual("16. kind",  result[2].kind, "ck");
        a.check("17. isDefault",  result[0].isDefault);
        a.check("18. isDefault", !result[1].isDefault);
        a.check("19. isDefault", !result[2].isDefault);
    }

    // Make one default
    testee.setDefault("c");
    {
        std::vector<HostTool::Info> result;
        testee.getAll(result);
        a.checkEqual("21. size", result.size(), 3U);
        std::sort(result.begin(), result.end(), Compare());
        a.check("22. isDefault", !result[0].isDefault);
        a.check("23. isDefault", !result[1].isDefault);
        a.check("24. isDefault", result[2].isDefault);
    }

    // Remove c
    testee.remove("c");
    {
        std::vector<HostTool::Info> result;
        testee.getAll(result);
        a.checkEqual("31. size", result.size(), 2U);
        std::sort(result.begin(), result.end(), Compare());
        a.checkEqual("32. id", result[0].id, "a");
        a.checkEqual("33. id", result[1].id, "b");
        a.check("34. isDefault", result[0].isDefault || result[1].isDefault);
    }
}

/** Test copy. */
AFL_TEST("server.host.HostTool:copy", a)
{
    TestHarness h;
    server::host::Session session;
    HostTool testee(session, h.root(), h.root().toolRoot(), HostTool::Tool);

    // Create a tool
    AFL_CHECK_SUCCEEDS(a("01. add"), testee.add("a", "", "", "kk"));
    AFL_CHECK_SUCCEEDS(a("02. set"), testee.set("a", "description", "Lengthy text..."));
    AFL_CHECK_SUCCEEDS(a("03. set"), testee.set("a", "docurl", "http://"));

    // Copy
    AFL_CHECK_SUCCEEDS(a("11. copy"), testee.copy("a", "x"));

    // Verify
    {
        std::vector<HostTool::Info> result;
        testee.getAll(result);
        a.checkEqual("21. size", result.size(), 2U);
        std::sort(result.begin(), result.end(), Compare());
        a.checkEqual("22. id", result[0].id, "a");
        a.checkEqual("23. id", result[1].id, "x");
        a.check("24. isDefault", result[0].isDefault || result[1].isDefault);
    }
    a.checkEqual("25. get", testee.get("x", "docurl"), "http://");
}

/** Test various error cases. */
AFL_TEST("server.host.HostTool:errors", a)
{
    TestHarness h;
    server::host::Session session;
    HostTool testee(session, h.root(), h.root().toolRoot(), HostTool::Tool);

    AFL_CHECK_SUCCEEDS(a("01. add"), testee.add("x", "", "", "k"));

    // Bad Id
    AFL_CHECK_THROWS(a("11. bad id"), testee.add("", "", "", "k"), std::exception);
    AFL_CHECK_THROWS(a("12. bad id"), testee.add("a b", "", "", "k"), std::exception);
    AFL_CHECK_THROWS(a("13. bad id"), testee.add("a\xC3\xB6", "", "", "k"), std::exception);
    AFL_CHECK_THROWS(a("14. bad id"), testee.set("", "k", "v"), std::exception);
    AFL_CHECK_THROWS(a("15. bad id"), testee.copy("x", ""), std::exception);

    // Bad Kind
    AFL_CHECK_THROWS(a("21. bad kind"), testee.add("a", "", "", ""), std::exception);
    AFL_CHECK_THROWS(a("22. bad kind"), testee.add("a", "", "", "a b"), std::exception);
    AFL_CHECK_THROWS(a("23. bad kind"), testee.add("a", "", "", "a-b"), std::exception);

    // Nonexistant
    AFL_CHECK_THROWS(a("31. nonexistant"), testee.copy("a", "b"), std::exception);
    AFL_CHECK_THROWS(a("32. nonexistant"), testee.setDefault("a"), std::exception);
    AFL_CHECK_THROWS(a("33. nonexistant"), testee.getDifficulty("a"), std::exception);
    AFL_CHECK_THROWS(a("34. nonexistant"), testee.clearDifficulty("a"), std::exception);
    AFL_CHECK_THROWS(a("35. nonexistant"), testee.setDifficulty("a", 99, true), std::exception);

    // Missing tool
    AFL_CHECK_THROWS(a("41. missing"), testee.add("a", "b", "c", "d"), std::exception);
}

/** Test difficulty access commands. */
AFL_TEST("server.host.HostTool:getDifficulty", a)
{
    TestHarness h;
    server::host::Session session;
    HostTool testee(session, h.root(), h.root().toolRoot(), HostTool::Tool);

    // Add a tool
    testee.add("t", "", "", "k");
    a.checkEqual("01. getDifficulty", testee.getDifficulty("t"), 0);

    // Set difficulty
    testee.setDifficulty("t", 33, true);
    a.checkEqual("11. getDifficulty", testee.getDifficulty("t"), 33);

    // Remove difficulty
    testee.clearDifficulty("t");
    a.checkEqual("21. getDifficulty", testee.getDifficulty("t"), 0);
}

/** Test difficulty computation. */
AFL_TEST("server.host.HostTool:computed-difficulty", a)
{
    TestHarness h;
    server::host::Session session;
    HostTool testee(session, h.root(), h.root().toolRoot(), HostTool::Tool);

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
    a.checkEqual("01. setDifficulty", n, 28);
    a.checkEqual("02. getDifficulty", testee.getDifficulty("easy"), 28);

    // Change the file to make it harder
    server::interface::FileBaseClient(h.hostFile()).putFile("dir/amaster.src",
                                                            "%amaster\n"
                                                            "planetcorerangesalternate=100,200\n"
                                                            "planetcorerangesusual=100,200\n"
                                                            "planetcoreusualfrequency=50\n"
                                                            "planetsurfaceranges=50,100\n");
    n = testee.setDifficulty("easy", afl::base::Nothing, true);
    a.checkEqual("11. setDifficulty", n, 126);
}
