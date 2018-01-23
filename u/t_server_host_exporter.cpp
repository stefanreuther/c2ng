/**
  *  \file u/t_server_host_exporter.cpp
  *  \brief Test for server::host::Exporter
  */

#include <unistd.h>
#include "server/host/exporter.hpp"

#include "t_server_host.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/textfile.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"
#include "server/file/filesystemhandler.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/file/utils.hpp"
#include "server/host/game.hpp"
#include "server/host/root.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::IntegerSetKey;
using afl::net::redis::StringKey;
using afl::net::redis::StringSetKey;
using afl::net::redis::Subtree;
using afl::string::Format;
using server::interface::FileBaseClient;

namespace {
    
    class TestHarness {
     public:
        TestHarness(const server::host::Configuration& config)
            : m_hostfile(),
              m_db(), m_null(), m_mail(m_null), m_runner(), m_fs(afl::io::FileSystem::getInstance()),
              m_root(m_db, m_hostfile, m_null, m_mail, m_runner, m_fs, config),
              m_workDirName()
            { createWorkDirectory(); }

        ~TestHarness()
            { removeWorkDirectory(); }

        server::host::Root& root()
            { return m_root; }

        afl::net::CommandHandler& db()
            { return m_db; }

        afl::io::FileSystem& fileSystem()
            { return m_fs; }

        afl::net::CommandHandler& hostFile()
            { return m_hostfile; }

        const String_t& getWorkDirName() const
            { return m_workDirName; }

        void addTool(String_t id, String_t cat);

     private:
        server::file::InternalFileServer m_hostfile;

        afl::net::redis::InternalDatabase m_db;
        afl::net::NullCommandHandler m_null;
        server::interface::MailQueueClient m_mail;
        util::ProcessRunner m_runner;
        afl::io::FileSystem& m_fs;
        server::host::Root m_root;
        String_t m_workDirName;

        void createWorkDirectory();
        void removeWorkDirectory();
    };

    String_t readFileContent(afl::io::FileSystem& fs, String_t name)
    {
        // We are only reading small files, so this can be simple
        afl::base::Ref<afl::io::Stream> f = fs.openFile(name, fs.OpenRead);
        uint8_t data[128];
        afl::base::Bytes_t bytes(data);
        bytes.trim(f->read(bytes));
        return afl::string::fromBytes(bytes);
    }
}

void
TestHarness::addTool(String_t id, String_t cat)
{
    // Files
    server::interface::FileBaseClient hostFile(m_hostfile);
    hostFile.createDirectory(Format("tools/%s", id));
    hostFile.putFile(Format("tools/%s/%s.txt", id, id), id);

    // Config
    HashKey(m_db, Format("prog:%s:prog:%s", cat, id)).stringField("path").set(Format("tools/%s", id));
    HashKey(m_db, Format("prog:%s:prog:%s", cat, id)).stringField("program").set(Format("%s.txt", id));
    HashKey(m_db, Format("prog:%s:prog:%s", cat, id)).stringField("kind").set(id);
    StringSetKey(m_db, Format("prog:%s:all", cat)).add(id);
}

void
TestHarness::createWorkDirectory()
{
    afl::base::Ref<afl::io::Directory> currentDirectory = m_fs.openDirectory(m_fs.getWorkingDirectoryName());
    int i = 0;
    while (1) {
        try {
            String_t name = Format("__test%d", ++i);
            afl::base::Ref<afl::io::DirectoryEntry> entry = currentDirectory->getDirectoryEntryByName(name);
            entry->createAsDirectory();
            m_workDirName = name;
            break;
        }
        catch (std::exception&) {
            if (i > 1000) {
                throw;
            }
        }
    }
}

void
TestHarness::removeWorkDirectory()
{
    // Sanity check. We're assigning it 6+ characters when it is successfully created
    if (m_workDirName.size() > 6) {
        server::file::FileSystemHandler handler(m_fs, m_workDirName);
        removeDirectoryContent(handler);

        m_fs.openDirectory(m_fs.getWorkingDirectoryName())->getDirectoryEntryByName(m_workDirName)->eraseNT();
    }
}


/********************************** Test *********************************/


/** Import/export test.
    This test is about moving game directories back and forth, hence it is rather complex to set up:
    it needs a game configured with all components to make sense.
    It is also hard to test from a system test, as the data it produces is transient.

    This test therefore builds the setup and then exports/imports it once.

    We don't currently have an InternalFileSystem, so this uses the real file system,
    and creates a temporary directory to work in. */
void
TestServerHostExporter::testIt()
{
    // Prepare:
    TestHarness h((server::host::Configuration()));

    // - static files
    server::interface::FileBaseClient hostFile(h.hostFile());
    hostFile.createDirectory("bin");
    hostFile.createDirectory("defaults");
    hostFile.createDirectory("games");
    hostFile.createDirectory("tools");
    hostFile.putFile("bin/runhost.sh", "script...");
    hostFile.putFile("defaults/default.ini", "def...");

    // - tools
    h.addTool("h", "host");
    h.addTool("m", "master");
    h.addTool("sl", "sl");
    h.addTool("t1", "tool");
    h.addTool("t2", "tool");

    // - game files
    hostFile.createDirectory("games/0042");
    hostFile.createDirectory("games/0042/data");
    hostFile.createDirectory("games/0042/in");
    hostFile.createDirectory("games/0042/out");
    hostFile.createDirectory("games/0042/backup");
    hostFile.putFile("games/0042/data/data.txt", "data file");
    hostFile.putFile("games/0042/data/dataold.txt", "old data file");
    hostFile.putFile("games/0042/in/in.txt", "in file");
    hostFile.putFile("games/0042/out/out.txt", "out file");
    hostFile.putFile("games/0042/backup/backup.txt", "backup file");

    // - game data
    StringKey(h.db(), "game:42:dir").set("games/0042");
    StringKey(h.db(), "game:42:name").set("Let's Rock");                // also serves as test for quoting...
    HashKey(h.db(), "game:42:settings").stringField("host").set("h");
    HashKey(h.db(), "game:42:settings").stringField("master").set("m");
    HashKey(h.db(), "game:42:settings").stringField("shiplist").set("sl");
    HashKey(h.db(), "game:42:settings").stringField("host").set("h");
    HashKey(h.db(), "game:42:settings").intField("turn").set(38);
    HashKey(h.db(), "game:42:toolkind").stringField("t1").set("t1");
    HashKey(h.db(), "game:42:toolkind").stringField("t2").set("t2");
    StringSetKey(h.db(), "game:42:tools").add("t1");
    StringSetKey(h.db(), "game:42:tools").add("t2");
    IntegerSetKey(h.db(), "game:all").add(42);

    // - game object
    server::host::Game game(h.root(), 42);

    // Action
    afl::io::FileSystem& fs = h.fileSystem();
    server::host::Exporter testee(h.hostFile(), fs, h.root().log());
    const String_t relativeName = testee.exportGame(game, h.root(), h.getWorkDirName());
    const String_t baseDirName = fs.makePathName(fs.getWorkingDirectoryName(), h.getWorkDirName());

    // Verification
    // - name must be given
    TS_ASSERT(relativeName.size() > 0);

    // - there must be a c2host.ini file. Read it into a map.
    std::map<String_t, String_t> ini;
    {
        afl::base::Ptr<afl::io::Stream> iniFile;
        TS_ASSERT_THROWS_NOTHING(iniFile = fs.openFile(fs.makePathName(fs.makePathName(baseDirName, relativeName), "c2host.ini"), fs.OpenRead).asPtr());

        afl::io::TextFile tf(*iniFile);
        String_t line;
        while (tf.readLine(line)) {
            String_t::size_type n = line.find('=');
            TS_ASSERT_DIFFERS(n, String_t::npos);
            String_t key(line, 0, n);
            String_t value(line, n+1);
            TS_ASSERT_EQUALS(ini.find(line), ini.end());
            ini.insert(std::make_pair(key, value));
        }
    }

    // - required keys in file
    TS_ASSERT_EQUALS(ini["game_settings_turn"], "38");
    TS_ASSERT_EQUALS(ini["game_name"], "Let\\'s\\ Rock");

    // - validate presence of tool files
    TS_ASSERT_DIFFERS(ini["game_host_path"], "");
    TS_ASSERT_EQUALS(ini["game_host"], "h");
    TS_ASSERT_EQUALS(readFileContent(fs, fs.makePathName(fs.makePathName(baseDirName, ini["game_host_path"]), "h.txt")), "h");

    TS_ASSERT_DIFFERS(ini["game_master_path"], "");
    TS_ASSERT_EQUALS(ini["game_master"], "m");
    TS_ASSERT_EQUALS(readFileContent(fs, fs.makePathName(fs.makePathName(baseDirName, ini["game_master_path"]), "m.txt")), "m");

    TS_ASSERT_DIFFERS(ini["game_sl_path"], "");
    TS_ASSERT_EQUALS(ini["game_sl"], "sl");
    TS_ASSERT_EQUALS(readFileContent(fs, fs.makePathName(fs.makePathName(baseDirName, ini["game_sl_path"]), "sl.txt")), "sl");

    TS_ASSERT_DIFFERS(ini["game_tool_t1_path"], "");
    TS_ASSERT_EQUALS(ini["game_tool_t1"], "t1");
    TS_ASSERT_EQUALS(readFileContent(fs, fs.makePathName(fs.makePathName(baseDirName, ini["game_tool_t1_path"]), "t1.txt")), "t1");

    TS_ASSERT_DIFFERS(ini["game_tool_t2_path"], "");
    TS_ASSERT_EQUALS(ini["game_tool_t2"], "t2");
    TS_ASSERT_EQUALS(readFileContent(fs, fs.makePathName(fs.makePathName(baseDirName, ini["game_tool_t2_path"]), "t2.txt")), "t2");

    // - validate presence of static files
    TS_ASSERT_EQUALS(readFileContent(fs, fs.makePathName(fs.makePathName(baseDirName, "bin"), "runhost.sh")), "script...");
    TS_ASSERT_EQUALS(readFileContent(fs, fs.makePathName(fs.makePathName(baseDirName, "defaults"), "default.ini")), "def...");

    // - validate presence of game files
    TS_ASSERT_EQUALS(readFileContent(fs, fs.makePathName(fs.makePathName(baseDirName, relativeName), "in/in.txt")), "in file");
    TS_ASSERT_EQUALS(readFileContent(fs, fs.makePathName(fs.makePathName(baseDirName, relativeName), "out/out.txt")), "out file");
    TS_ASSERT_EQUALS(readFileContent(fs, fs.makePathName(fs.makePathName(baseDirName, relativeName), "data/data.txt")), "data file");

    // Update in/out/data
    fs.openFile(fs.makePathName(fs.makePathName(baseDirName, relativeName), "in/in2.txt"), fs.Create)->fullWrite(afl::string::toBytes("created in"));
    fs.openFile(fs.makePathName(fs.makePathName(baseDirName, relativeName), "out/out2.txt"), fs.Create)->fullWrite(afl::string::toBytes("created out"));
    fs.openFile(fs.makePathName(fs.makePathName(baseDirName, relativeName), "data/data2.txt"), fs.Create)->fullWrite(afl::string::toBytes("created data"));
    fs.openFile(fs.makePathName(fs.makePathName(baseDirName, relativeName), "backup/backup2.txt"), fs.Create)->fullWrite(afl::string::toBytes("created backup"));

    fs.openDirectory(fs.makePathName(fs.makePathName(baseDirName, relativeName), "data"))->erase("dataold.txt");

    // Synchronize back
    TS_ASSERT_THROWS_NOTHING(testee.importGame(game, h.root(), h.getWorkDirName()));

    // Verify
    TS_ASSERT_EQUALS(hostFile.getFile("games/0042/data/data.txt"), "data file");
    TS_ASSERT_THROWS(hostFile.getFile("games/0042/data/dataold.txt"), std::exception);
    TS_ASSERT_EQUALS(hostFile.getFile("games/0042/data/data2.txt"), "created data");
    TS_ASSERT_EQUALS(hostFile.getFile("games/0042/out/out2.txt"), "created out");
    TS_ASSERT_EQUALS(hostFile.getFile("games/0042/in/in2.txt"), "created in");
    TS_ASSERT_EQUALS(hostFile.getFile("games/0042/backup/backup2.txt"), "created backup");
    TS_ASSERT_EQUALS(hostFile.getFile("games/0042/backup/backup.txt"), "backup file");
}

/** Test how backups are imported on import. */
void
TestServerHostExporter::testUnpackBackups()
{
    // Prepare:
    server::host::Configuration config;
    config.unpackBackups = true;
    TestHarness h(config);

    // - static files
    server::interface::FileBaseClient hostFile(h.hostFile());
    hostFile.createDirectory("bin");
    hostFile.createDirectory("defaults");
    hostFile.createDirectory("games");

    // - game files
    hostFile.createDirectory("games/0042");
    hostFile.createDirectory("games/0042/data");
    hostFile.createDirectory("games/0042/in");
    hostFile.createDirectory("games/0042/out");
    hostFile.createDirectory("games/0042/backup");
    hostFile.createDirectory("games/0042/backup/other");
    hostFile.putFile("games/0042/backup/backup.txt", "backup file");
    hostFile.putFile("games/0042/backup/other/other.txt", "other file");

    // - game data
    StringKey(h.db(), "game:42:dir").set("games/0042");
    StringKey(h.db(), "game:42:name").set("Let's Rock");                // also serves as test for quoting...
    HashKey(h.db(), "game:42:settings").intField("turn").set(38);
    IntegerSetKey(h.db(), "game:all").add(42);

    // - game object
    server::host::Game game(h.root(), 42);

    // Export to initialize
    afl::io::FileSystem& fs = h.fileSystem();
    server::host::Exporter testee(h.hostFile(), fs, h.root().log());
    const String_t relativeName = testee.exportGame(game, h.root(), h.getWorkDirName());
    const String_t baseDirName = fs.makePathName(fs.getWorkingDirectoryName(), h.getWorkDirName());

    // Place backups
    // - a tarball that contains a single file "a.txt" containing "a file"
    static const uint8_t A_TAR_GZ[] = {
        0x1f, 0x8b, 0x08, 0x00, 0x7f, 0xf5, 0xd0, 0x59, 0x00, 0x03, 0xed, 0xce,
        0x41, 0x0a, 0x83, 0x30, 0x14, 0x84, 0xe1, 0x77, 0x94, 0x9c, 0x40, 0xf2,
        0x9a, 0xe4, 0x79, 0x9e, 0x2c, 0x22, 0x08, 0xe2, 0xa2, 0xa6, 0xe0, 0xf1,
        0xd5, 0xd2, 0x45, 0x37, 0x45, 0x5c, 0x84, 0x22, 0xfc, 0xdf, 0x66, 0x16,
        0x33, 0x8b, 0xc9, 0x5d, 0x5d, 0xab, 0xb4, 0xe5, 0x77, 0x16, 0xe3, 0x91,
        0xda, 0x27, 0xff, 0x9d, 0x1f, 0x26, 0x1a, 0xd4, 0xa2, 0xf6, 0x8f, 0x64,
        0x41, 0xbc, 0x6a, 0xb0, 0x24, 0xce, 0x37, 0xfe, 0xf5, 0xf6, 0x5a, 0x6a,
        0x7e, 0x3a, 0x27, 0x4b, 0x2d, 0x43, 0x9e, 0x7f, 0xef, 0xce, 0xfa, 0x9b,
        0xca, 0x6e, 0x18, 0xa7, 0xf2, 0xef, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xab, 0x36, 0x30,
        0xf8, 0xfa, 0x86, 0x00, 0x28, 0x00, 0x00
    };

    // - a tarball that contains two files "./1.txt" and "./2.txt" (and a directory "./")
    static const uint8_t TWO_TAR_GZ[] = {
        0x1f, 0x8b, 0x08, 0x00, 0xc2, 0xf5, 0xd0, 0x59, 0x00, 0x03, 0xed, 0xd4,
        0x31, 0x0e, 0x02, 0x21, 0x10, 0x85, 0xe1, 0x39, 0x0a, 0x27, 0x00, 0x66,
        0x77, 0x80, 0xf3, 0x6c, 0xa1, 0xa5, 0x85, 0x8b, 0x89, 0xc7, 0x77, 0xd7,
        0x68, 0xb4, 0x51, 0x0b, 0x83, 0x68, 0xfc, 0xbf, 0x66, 0x0a, 0x48, 0x78,
        0xc9, 0xcb, 0xe0, 0x83, 0x34, 0x17, 0x17, 0x25, 0xa5, 0x75, 0x6a, 0x49,
        0xf1, 0x7e, 0x5e, 0x89, 0x8e, 0x9a, 0x4d, 0xcb, 0x90, 0x8b, 0x4a, 0xd4,
        0x98, 0x2c, 0x89, 0x4b, 0xed, 0xa3, 0x89, 0x1c, 0xe6, 0x3a, 0xed, 0x9d,
        0x93, 0xb9, 0x6e, 0xb6, 0xd3, 0xee, 0xf1, 0xbd, 0x57, 0xe7, 0x3f, 0xca,
        0x07, 0xf5, 0xf5, 0x58, 0x9b, 0xbe, 0xb1, 0x16, 0x9c, 0xcd, 0x9e, 0xf4,
        0xaf, 0xb7, 0xfe, 0x73, 0x5e, 0xfa, 0x57, 0x33, 0x15, 0x17, 0x9b, 0xa6,
        0xba, 0xf8, 0xf3, 0xfe, 0xb5, 0x77, 0x00, 0x74, 0xe5, 0xc3, 0xf0, 0x5d,
        0xfb, 0x7f, 0xfe, 0xff, 0xd5, 0xc6, 0xcc, 0xfe, 0x7f, 0xc2, 0xd0, 0x3b,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x78, 0xdb, 0x09, 0xbb, 0x7b, 0x2f, 0x52, 0x00, 0x28, 0x00, 0x00
    };
    fs.openFile(fs.makePathName(fs.makePathName(baseDirName, relativeName), "backup/pre.tar.gz"), fs.Create)->fullWrite(A_TAR_GZ);
    fs.openFile(fs.makePathName(fs.makePathName(baseDirName, relativeName), "backup/post.tgz"), fs.Create)->fullWrite(A_TAR_GZ);
    fs.openFile(fs.makePathName(fs.makePathName(baseDirName, relativeName), "backup/other.tgz"), fs.Create)->fullWrite(TWO_TAR_GZ);

    // Synchronize back
    TS_ASSERT_THROWS_NOTHING(testee.importGame(game, h.root(), h.getWorkDirName()));

    // Verify
    TS_ASSERT_EQUALS(hostFile.getFile("games/0042/backup/pre/a.txt"), "a file");
    TS_ASSERT_EQUALS(hostFile.getFile("games/0042/backup/post/a.txt"), "a file");
    TS_ASSERT_EQUALS(hostFile.getFile("games/0042/backup/other/1.txt"), "1");
    TS_ASSERT_EQUALS(hostFile.getFile("games/0042/backup/other/2.txt"), "2");
    TS_ASSERT_THROWS(hostFile.getFile("games/0042/backup/other/other.txt"), std::exception);
}

