/**
  *  \file u/t_game_proxy_pluginmanagerproxy.cpp
  *  \brief Test for game::proxy::PluginManagerProxy
  */

#include <cstring>
#include "game/proxy/pluginmanagerproxy.hpp"

#include "t_game_proxy.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"

namespace {
    class Receiver {
     public:
        Receiver()
            : m_list(), m_details("?", "?", util::plugin::Manager::NotLoaded), m_numLists(), m_numDetails()
            { }

        void onList(const game::proxy::PluginManagerProxy::Infos_t& list)
            {
                m_list = list;
                ++m_numLists;
            }

        void onDetails(const game::proxy::PluginManagerProxy::Details_t& d)
            {
                m_details = d;
                ++m_numDetails;
            }

        void connect(game::proxy::PluginManagerProxy& proxy)
            {
                proxy.sig_list.add(this, &Receiver::onList);
                proxy.sig_details.add(this, &Receiver::onDetails);
            }

        game::proxy::PluginManagerProxy::Infos_t m_list;
        game::proxy::PluginManagerProxy::Details_t m_details;
        int m_numLists;
        int m_numDetails;
    };
}

/** Test general information requests. */
void
TestGameProxyPluginManagerProxy::testRequest()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    t.session().plugins().addNewPlugin(new util::plugin::Plugin("A"));
    t.session().plugins().addNewPlugin(new util::plugin::Plugin("B"));

    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    Receiver recv;
    recv.connect(testee);

    // requestList()
    testee.requestList();
    t.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(recv.m_numLists, 1);
    TS_ASSERT_EQUALS(recv.m_list.size(), 2U);
    TS_ASSERT_EQUALS(recv.m_list[0].id, "A");
    TS_ASSERT_EQUALS(recv.m_list[1].id, "B");

    // requestDetails()
    testee.requestDetails("B");
    t.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(recv.m_numDetails, 1);
    TS_ASSERT_EQUALS(recv.m_details.id, "B");
}

/** Test request debouncing.
    If we send multiple requests, ideally only a single response shall arrive (no response queueing). */
void
TestGameProxyPluginManagerProxy::testDebounce()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    t.session().plugins().addNewPlugin(new util::plugin::Plugin("A"));
    t.session().plugins().addNewPlugin(new util::plugin::Plugin("B"));

    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    Receiver recv;
    recv.connect(testee);

    // Request varying data
    for (int i = 1; i < 100; ++i) {
        testee.requestDetails("A");
        testee.requestDetails("B");
    }

    // PluginManagerProxy will perform an unspecified number of requests before producing a callback.
    // However, we want it to be significantly fewer than the number of requests.
    for (int i = 1; i < 10; ++i) {
        t.sync();
        ind.processQueue();
    }

    // Number of responses shall be even lower (ideally, 1).
    TS_ASSERT_LESS_THAN(recv.m_numDetails, 3);
    TS_ASSERT_EQUALS(recv.m_details.id, "B");
}

/** Test installation, happy case. */
void
TestGameProxyPluginManagerProxy::testInstall()
{
    // A file system containing the plugin
    const char*const PLUGIN_CONTENT =
        "name = TestPlugin\n"
        "description = Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\n"
        "description = Duis sem velit, ultrices et, fermentum auctor, rhoncus ut, ligula.\n"
        "helpfile = h.xml\n";
    const char*const HELP_CONTENT = "<help />";

    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes(PLUGIN_CONTENT));
    fs.openFile("/in/h.xml", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes(HELP_CONTENT));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.c2p");
    TS_ASSERT_EQUALS(info.isValid, true);
    TS_ASSERT_EQUALS(info.isUpdate, false);
    TS_ASSERT_EQUALS(info.ambiguity, util::plugin::Installer::NoPlugin);
    // errorMessage not relevant
    TS_ASSERT_EQUALS(info.fileName, "/in/t.c2p");
    TS_ASSERT_EQUALS(info.fileTitle, "t.c2p");
    // altName, altTitle not relevant
    TS_ASSERT_EQUALS(info.pluginId, "T");
    TS_ASSERT_EQUALS(info.pluginName, "TestPlugin");
    TS_ASSERT_EQUALS(info.pluginDescription, "Lorem ipsum dolor sit amet, consectetuer adipiscing elit.");  // only first line reported here
    TS_ASSERT(!info.conflicts.isValid());

    TS_ASSERT_THROWS(fs.openFile("/p/t/h.xml", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);

    // Perform the installation; verify result
    game::proxy::PluginManagerProxy::InstallResult result = testee.doInstall(ind);
    TS_ASSERT_EQUALS(result.isValid, true);
    TS_ASSERT_EQUALS(result.pluginId, "T");

    // Verify that plugin was installed
    // - content needs to be present
    uint8_t found[100];
    afl::base::ConstBytes_t foundBytes(found);
    TS_ASSERT_THROWS_NOTHING(foundBytes.trim(fs.openFile("/p/t/h.xml", afl::io::FileSystem::OpenRead)->read(found)));
    TS_ASSERT_EQUALS(foundBytes.size(), std::strlen(HELP_CONTENT));
    TS_ASSERT_SAME_DATA(foundBytes.unsafeData(), HELP_CONTENT, std::strlen(HELP_CONTENT));

    // - plugin file needs to be present (don't check content, it can be rewritten)
    TS_ASSERT_THROWS_NOTHING(fs.openFile("/p/t.c2p", afl::io::FileSystem::OpenRead));
}

/** Test installation, error: no directory set.
    Installation fails immediately.
    This is not a relevant error condition. */
void
TestGameProxyPluginManagerProxy::testInstallNoDir()
{
    // A file system containing the plugin
    const char*const PLUGIN_CONTENT = "name = TestPlugin\n";

    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes(PLUGIN_CONTENT));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    // no setPluginDirectoryName()

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.c2p");
    TS_ASSERT_EQUALS(info.isValid, false);
    // We don't produce any specific error message here
}

/** Test installation, error: file does not exist.
    Installation fails.
    The error message should be provided to the user. */
void
TestGameProxyPluginManagerProxy::testInstallNoFile()
{
    // Session with no file system
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.c2p");
    TS_ASSERT_EQUALS(info.isValid, false);
    TS_ASSERT_DIFFERS(info.errorMessage, ""); // should be ENOENT message from file system
}

/** Test installation, error: file not understood.
    Installation fails.
    This is the normal trigger for an failure without error message. */
void
TestGameProxyPluginManagerProxy::testInstallBadFile()
{
    // A file system containing a file
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.xyz", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("whatever"));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.xyz");
    TS_ASSERT_EQUALS(info.isValid, false);
    // We don't produce any specific error message here
}

/** Test installation: "provides" conflict (provided feature already exists).
    Conflict is reported. */
void
TestGameProxyPluginManagerProxy::testInstallConflict()
{
    // A file system containing the plugin
    const char*const PLUGIN_CONTENT =
        "name = TestPlugin\n"
        "provides = QQQ\n";

    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes(PLUGIN_CONTENT));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");
    t.session().plugins().addNewPlugin(new util::plugin::Plugin("QQQ"));

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.c2p");
    TS_ASSERT_EQUALS(info.isValid, true);
    TS_ASSERT_EQUALS(info.isUpdate, false);

    // Must report a nonempty error message that mentions QQQ
    TS_ASSERT(info.conflicts.isValid());
    TS_ASSERT_DIFFERS(*info.conflicts.get(), "");
    TS_ASSERT_DIFFERS(info.conflicts.get()->find("QQQ"), String_t::npos);
}

/** Test installation: "requires" conflict (required feature does not exist).
    Conflict is reported. */
void
TestGameProxyPluginManagerProxy::testInstallConflict2()
{
    // A file system containing the plugin
    const char*const PLUGIN_CONTENT =
        "name = TestPlugin\n"
        "requires = ZZZ\n";

    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes(PLUGIN_CONTENT));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.c2p");
    TS_ASSERT_EQUALS(info.isValid, true);
    TS_ASSERT_EQUALS(info.isUpdate, false);

    // Must report a nonempty error message that mentions QQQ
    TS_ASSERT(info.conflicts.isValid());
    TS_ASSERT_DIFFERS(*info.conflicts.get(), "");
    TS_ASSERT_DIFFERS(info.conflicts.get()->find("ZZZ"), String_t::npos);
}

/** Test installation: single alternative.
    Alternative is reported. */
void
TestGameProxyPluginManagerProxy::testInstallOneAlternative()
{
    // File system content
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("..."));
    fs.openFile("/in/h.res", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("..."));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/h.res");
    TS_ASSERT_EQUALS(info.isValid, true);
    TS_ASSERT_EQUALS(info.isUpdate, false);
    TS_ASSERT_EQUALS(info.ambiguity, util::plugin::Installer::OnePlugin);
    // errorMessage not relevant
    TS_ASSERT_EQUALS(info.fileName, "/in/h.res");
    TS_ASSERT_EQUALS(info.fileTitle, "h.res");
    TS_ASSERT_EQUALS(info.altName, "/in/t.c2p");
    TS_ASSERT_EQUALS(info.altTitle, "t.c2p");
    TS_ASSERT_EQUALS(info.pluginId, "H");
    TS_ASSERT(!info.conflicts.isValid());
}

/** Test installation: multiple alternatives.
    Availability of alternatives is reported. */
void
TestGameProxyPluginManagerProxy::testInstallMultipleAlternatives()
{
    // File system content
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("..."));
    fs.openFile("/in/s.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("..."));
    fs.openFile("/in/h.res", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("..."));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/h.res");
    TS_ASSERT_EQUALS(info.isValid, true);
    TS_ASSERT_EQUALS(info.isUpdate, false);
    TS_ASSERT_EQUALS(info.ambiguity, util::plugin::Installer::MultiplePlugins);
    // errorMessage not relevant
    TS_ASSERT_EQUALS(info.fileName, "/in/h.res");
    TS_ASSERT_EQUALS(info.fileTitle, "h.res");
    // altName, altTitle not relevant
    TS_ASSERT_EQUALS(info.pluginId, "H");
    TS_ASSERT(!info.conflicts.isValid());
}

/** Test installation: missing payload file.
    This fails the installation with an exception. */
void
TestGameProxyPluginManagerProxy::testInstallMissingFile()
{
    // A file system containing the plugin
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("helpfile = h.xml\n"));
    // no h.xml

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.c2p");
    TS_ASSERT_EQUALS(info.isValid, true);

    // Perform the installation; verify result. Error message must reference the missing file name.
    game::proxy::PluginManagerProxy::InstallResult result = testee.doInstall(ind);
    TS_ASSERT_EQUALS(result.isValid, false);
    TS_ASSERT_DIFFERS(result.errorMessage.find("h.xml"), String_t::npos);

    // Verify that plugin was not installed
    TS_ASSERT_THROWS(fs.openFile("/p/t.c2p", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
}

/** Test installation: bad sequence (cancellation). */
void
TestGameProxyPluginManagerProxy::testInstallBadSequence()
{
    // A file system containing the plugin
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("name = plugin\n"));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.c2p");
    TS_ASSERT_EQUALS(info.isValid, true);

    // Sequence violation
    testee.cancelInstallation();

    // Try to perform the installation; must fail.
    game::proxy::PluginManagerProxy::InstallResult result = testee.doInstall(ind);
    TS_ASSERT_EQUALS(result.isValid, false);

    // Verify that plugin was not installed
    TS_ASSERT_THROWS(fs.openFile("/p/t.c2p", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
}

/** Test remove, happy case. */
void
TestGameProxyPluginManagerProxy::testRemove()
{
    // A file system containing installed plugins
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/p");
    fs.createDirectory("/p/t");
    fs.createDirectory("/p/q");
    fs.openFile("/p/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("name = plugin\nhelpfile = h.xml\n"));
    fs.openFile("/p/t/h.xml", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("<help />"));
    fs.openFile("/p/q.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("name = other\n"));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().plugins().findPlugins(*fs.openDirectory("/p"));
    t.session().setPluginDirectoryName("/p");

    // Prepare
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::RemoveResult preResult = testee.prepareRemove(ind, "T");
    TS_ASSERT_EQUALS(preResult.isValid, true);
    TS_ASSERT_THROWS_NOTHING(fs.openFile("/p/t.c2p", afl::io::FileSystem::OpenRead));  // file still exists

    // Remove
    game::proxy::PluginManagerProxy::RemoveResult result = testee.doRemove(ind, "T");
    TS_ASSERT_EQUALS(result.isValid, true);

    // Files gone
    TS_ASSERT_THROWS(fs.openFile("/p/t.c2p", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
    TS_ASSERT_THROWS(fs.openFile("/p/t/h.xml", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);

    // Only plugin Q remains
    std::vector<util::plugin::Plugin*> plugins;
    t.session().plugins().enumPlugins(plugins, true);
    TS_ASSERT_EQUALS(plugins.size(), 1U);
    TS_ASSERT_EQUALS(plugins[0]->getId(), "Q");
}

/** Test remove, plugin is required by someone else. */
void
TestGameProxyPluginManagerProxy::testRemoveDepend()
{
    // A file system containing installed plugins
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/p");
    fs.createDirectory("/p/ttt");
    fs.createDirectory("/p/q");
    fs.openFile("/p/ttt.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("name = plug\nrequires = q\n"));
    fs.openFile("/p/q.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("name = other\n"));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().plugins().findPlugins(*fs.openDirectory("/p"));
    t.session().setPluginDirectoryName("/p");

    // Prepare
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::RemoveResult preResult = testee.prepareRemove(ind, "Q");
    TS_ASSERT_EQUALS(preResult.isValid, false);
    TS_ASSERT_DIFFERS(preResult.errorMessage.find("TTT"), String_t::npos);
}

/** Test remove, not all files exist. */
void
TestGameProxyPluginManagerProxy::testRemoveMissing()
{
    // A file system containing installed plugins
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/p");
    fs.createDirectory("/p/t");
    fs.openFile("/p/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("name = plugin\nhelpfile = h.xml\n"));
    // no h.xml file

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().plugins().findPlugins(*fs.openDirectory("/p"));
    t.session().setPluginDirectoryName("/p");

    // Prepare
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::RemoveResult preResult = testee.prepareRemove(ind, "T");
    TS_ASSERT_EQUALS(preResult.isValid, true);

    // Remove
    game::proxy::PluginManagerProxy::RemoveResult result = testee.doRemove(ind, "T");
    TS_ASSERT_EQUALS(result.isValid, false);
}

/** Test remove, desired name does not exist. */
void
TestGameProxyPluginManagerProxy::testUninstallBadName()
{
    // A file system with no installed plugins
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/p");

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().plugins().findPlugins(*fs.openDirectory("/p"));
    t.session().setPluginDirectoryName("/p");

    // Prepare reports failure
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::RemoveResult preResult = testee.prepareRemove(ind, "T");
    TS_ASSERT_EQUALS(preResult.isValid, false);

    // Remove reports failure
    game::proxy::PluginManagerProxy::RemoveResult result = testee.doRemove(ind, "T");
    TS_ASSERT_EQUALS(result.isValid, false);
}

